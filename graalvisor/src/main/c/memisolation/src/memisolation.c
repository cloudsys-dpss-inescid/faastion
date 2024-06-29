#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>
#include "seccomp.h" // Note: constants copied from '/usr/include/linux/seccomp.h'
#include "utils/appmap.h"
#include "helpers/helpers.h"
#include "memisolation.h"

#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))
#define NUM_DOMAINS 16
#define BUFFER_SIZE 1024
#define NUM_PROCESSES 20

/* The following is the x86-64-specific BPF boilerplate code for checking
   that the BPF program is running on the right architecture. */

#define X86_64_CHECK_ARCH \
    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
	    (offsetof(struct seccomp_data, arch))), \
	    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0), \
	    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL), \
	    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
		    (offsetof(struct seccomp_data, nr)))

__thread int domain = 0;

atomic_int counter = ATOMIC_VAR_INIT(0);

static int eager_mpk = 0;
static int eager_perms = 0;

/* File descriptors */
struct Supervisor supervisors[NUM_DOMAINS];

/* Application map */
AppNode* apps = NULL;

/* Process IDs */
volatile atomic_int procIDs[NUM_PROCESSES];

/* Thread count per domain */
int threadCount[NUM_DOMAINS];

/* Application cache */
char* cache[NUM_DOMAINS];

/* seccomp system call */
static int seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(SYS_seccomp, operation, flags, args);
}

/* MPK domains */
static void set_permissions(const char* id, int pkey)
{
    MRNode* current = get_regions(apps, id);
    while (current) {
		if (pkey_mprotect(current->region.address, current->region.size, current->region.flags, pkey) == -1) {
            fprintf(stderr, "pkey_mprotect error\n");
            exit(EXIT_FAILURE);
        }
		current = current->next;
    }
}

void insert_memory_regions(char* id, const char* path, pthread_mutex_t mutex) {
    get_memory_regions(apps, id, path, mutex);
}

/* Supervisors */

/* Installs a seccomp filter that blocks all pkey related system calls;
   the filter generates user-space notifications (SECCOMP_RET_USER_NOTIF)
   on all other system calls. */

static int install_notify_filter()
{   
    struct sock_filter filter[] = {
		X86_64_CHECK_ARCH,

		/* pkey_alloc(2) and pkey_free(2) trigger KILL signal */

		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_alloc, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_free, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

		/* pkey_mprotect(2), mmap(2), clone3(2) and exit(2) 
		trigger notifications to user-space supervisor */

		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_mprotect, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_munmap, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_mmap, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_clone3, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_exit, 0, 1),
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

		/* Every other system call is allowed */

		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
		.len = ARRAY_SIZE(filter),
		.filter = filter,
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
		err(EXIT_FAILURE, "prctl");

    /* Install the filter with the SECCOMP_FILTER_FLAG_NEW_LISTENER flag;
       as a result, seccomp() returns a notification file descriptor. */

    int nfd = seccomp(SECCOMP_SET_MODE_FILTER,
	    SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);

    if (nfd == -1)
		err(EXIT_FAILURE, "seccomp-install-notify-filter");

    return nfd;
}

static void prep_env(const char* application)
{
    SEC_DBM("\t[S%d]: preparing environment.", domain);
    struct Supervisor* sp = &supervisors[domain];

    sp->status = ACTIVE;
    sp->execution = MANAGED;

    // Get application in cache
    char* cached = cache[domain];

    if (eager_perms) {
        if (strcmp(cached, application)) {
            strcpy(cache[domain], application);
        } 
        set_permissions(application, domain);
	}
    else {
        // If application in cache differs from application assigned to supervisor
        if (strcmp(cached, application)) {
            // 1. Remove domain permissions of previous application if any
            if (strcmp(cached, "")) {
                set_permissions(cached, 1);
            }
            // 2. Insert new application in cache and give permissions
            strcpy(cache[domain], application);
            set_permissions(application, domain);
        }
        // Else continue normal execution since application is the same
	}

    sp->execution = NATIVE;
}

void reset_env(const char* application, int isLast)
{
    if (eager_mpk && !isLast) {
		return;
    }

    SEC_DBM("\t[S%d]: application finished.", domain);
    struct Supervisor* sp = &supervisors[domain];

	if (eager_mpk && sp->status) {
		atomic_fetch_sub(&counter, 1);
		threadCount[domain] = 0;
	}
    if (eager_perms) {
        sp->execution = MANAGED;
        set_permissions(application, 1);    
	}
	
	if(sp->fd != 0){
		add_fd(sp->fd);
	}
    sp->status = DONE;
}

static void assign_supervisor(const char* app, int* fd)
{
    SEC_DBM("\t[S%d]: application assigned: %s.", domain, app);
    struct Supervisor* sp = &supervisors[domain];

    // install seccomp filter
    if (!*fd) {
		SEC_DBM("\t[S%d]: Installing filter.", domain);
		*fd = install_notify_filter();
    }
  
	// even if filter is installed we have to update supervisor's file descriptor
    // because another thread with another filter could have been used before
    sp->fd = *fd;

	if(sp->fd != 0){
	remove_fd(sp->fd);
	}
	  
    strcpy(sp->app, app);

    prep_env(app);
}

/* Returns 1 (True) if app is still in cache;
   Returns 0 (False) otherwise. */

int is_app_cached(const char* app) {
    return !strcmp((const char *)cache[domain], app);
}

/* TODO: If app is executing, assign domain to -1 */
int is_app_executing(const char* app) {
    for (int i = 2; i < NUM_DOMAINS; i++) {
        if (!strcmp((const char *)cache[i], app) && __sync_bool_compare_and_swap(&threadCount[i], 1, 1)) {
            return 1;
        }
    }
    return 0;
}

/* Domain Management algorithm */
void find_domain_eager(const char* app) {
	while (1) {
		if (domain == 0 || !is_app_cached(app) || !__sync_bool_compare_and_swap(&threadCount[domain], 0, 1)) {
			for (int i = 2; i < NUM_DOMAINS; ++i) {
				if (__sync_bool_compare_and_swap(&threadCount[i], 0, 1)) {
					atomic_fetch_add(&counter, 1);
					domain = i;
					return;
				}
			}
			// If no empty domains found, wait briefly
			fprintf(stderr, "Domains exhausted, waiting 1 ms\n");
			usleep(1000);
		} else {
			SEC_DBM("\t[S%d]: App is cached", domain);
			atomic_fetch_add(&counter, 1);
			return;
		}
	}
}

void find_domain(const char* app, int *fd) {
    for (int i = 2; i < NUM_DOMAINS; ++i) {
		if (__sync_bool_compare_and_swap(&threadCount[i], 0, 1)) {
			domain = i;
			atomic_fetch_add(&counter, 1);
			assign_supervisor(app, fd);
			return;
		}
    }
    domain = -1;
	fprintf(stderr, "No domain available, proceeding with LPI\n");
}

void acquire_domain(const char* app, int *fd) {
    if (eager_mpk) {
        assign_supervisor(app, fd);
        return;
	}

    /*
       If domain is set to 0, it indicates that this is the first invocation of the application.
       If domain is set to -1, it indicates that during the previous execution of this application, no domains were available.
    */
    if (domain <= 0 || !is_app_cached(app) || !__sync_bool_compare_and_swap(&threadCount[domain], 0, 1)) {
		find_domain(app, fd);
    } else {
		SEC_DBM(stderr,"\t[S%d]: App is cached", domain);
		atomic_fetch_add(&counter, 1);
		assign_supervisor(app, fd);
    }
}

/* Seccomp */

/* Check that the notification ID provided by a SECCOMP_IOCTL_NOTIF_RECV
   operation is still valid. It will no longer be valid if the target
   has terminated or is no longer blocked in the system call that
   generated the notification (because it was interrupted by a signal).

   This operation can be used when doing such things as accessing
   /proc/PID files in the target process in order to avoid TOCTOU race
   conditions where the PID that is returned by SECCOMP_IOCTL_NOTIF_RECV
   terminates and is reused by another process. */

static bool cookie_is_valid(int notifyFd, uint64_t id)
{
    return ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_ID_VALID, &id) == 0;
}

/* Allocate buffers for the seccomp user-space notification request and
   response structures. It is the caller's responsibility to free the
   buffers returned via 'req' and 'resp'. */

static void alloc_seccomp_notif_buffers(struct seccomp_notif **req,
	struct seccomp_notif_resp **resp,
	struct seccomp_notif_sizes *sizes)
{
    size_t  resp_size;

    /* Discover the sizes of the structures that are used to receive
       notifications and send notification responses, and allocate
       buffers of those sizes. */

    if (seccomp(SECCOMP_GET_NOTIF_SIZES, 0, sizes) == -1)
		err(EXIT_FAILURE, "[S%d]: seccomp-SECCOMP_GET_NOTIF_SIZES", domain);

    *req = malloc(sizes->seccomp_notif);
    if (*req == NULL)
		err(EXIT_FAILURE, "[S%d]: malloc-seccomp_notif", domain);

    /* When allocating the response buffer, we must allow for the fact
       that the user-space binary may have been built with user-space
       headers where 'struct seccomp_notif_resp' is bigger than the
       response buffer expected by the (older) kernel. Therefore, we
       allocate a buffer that is the maximum of the two sizes. This
       ensures that if the supervisor places bytes into the response
       structure that are past the response size that the kernel expects,
       then the supervisor is not touching an invalid memory location. */

    resp_size = sizes->seccomp_notif_resp;
    if (sizeof(struct seccomp_notif_resp) > resp_size)
		resp_size = sizeof(struct seccomp_notif_resp);

    *resp = malloc(resp_size);
    if (*resp == NULL)
		err(EXIT_FAILURE, "[S%d]: malloc-seccomp_notif_resp", domain);

}

static void handle_pkey_mprotect(struct seccomp_notif *req, struct seccomp_notif_resp *resp) 
{
    SEC_DBM("\t----pkey_mprotect syscall----");
    struct Supervisor* sp = &supervisors[domain];

    // Native execution notification
    if (!sp->execution) {
		// KILL
		resp->error = -errno;
		SEC_DBM("\t[S%d]: Native call trying to change permissions, killing...", domain);
		return;
    }

    int prot = pkey_mprotect((void *)req->data.args[0], req->data.args[1], req->data.args[2], req->data.args[3]);
    if (prot == -1) {
		/* If pkey_mprotect() failed in the supervisor, pass the error
		back to the target */

		resp->error = -errno;
		SEC_DBM("\t[S%d]: failure! (errno = %d; %s)\n", domain, errno,
			strerror(errno));
    }
    else {
		resp->error = 0;          /* "Success" */
		resp->val = (__s64)prot;  /* return value of pkey_mprotect() in target */

		SEC_DBM("\t[S%d]: success! spoofed return = %d; spoofed val = %lld\n",
			domain, prot, resp->val);
    }
}

static void handle_mmap(struct seccomp_notif *req, struct seccomp_notif_resp *resp) 
{
    SEC_DBM("\t----mmap syscall----");

    void *mapped_mem = mmap((void *)req->data.args[0], req->data.args[1],
	    req->data.args[2], req->data.args[3],
	    req->data.args[4], req->data.args[5]);

    if (mapped_mem == MAP_FAILED) {
		/* If mmap() failed in the supervisor, pass the error
		back to the target */

		resp->error = -errno;
		SEC_DBM("\t[S%d]: failure! (errno = %d; %s)\n", domain, errno,
			strerror(errno));
    }
    else {
		if (pkey_mprotect(mapped_mem, req->data.args[1], req->data.args[2], domain) == -1) {
			resp->error = 1;            /* random value different than 0 */
			perror("pkey_mprotect");
			return;
		}

		/* assign allocated memory to application */
		void* address = mapped_mem;
		size_t size = req->data.args[1];
		int flags = req->data.args[2];
		insert_MRNode(apps, supervisors[domain].app, address, size, flags);

		resp->error = 0;                /* "Success" */
		resp->val = (__s64)mapped_mem;  /* return value of mmap() in target */

		SEC_DBM("\t[S%d]: success! spoofed return = %p; spoofed val = %lld\n",
			domain, mapped_mem, resp->val);
    }
}

static void handle_munmap(struct seccomp_notif *req, struct seccomp_notif_resp *resp)
{
    SEC_DBM("\t----munmmap syscall----");

    remove_MRNode(apps, supervisors[domain].app, (void *)req->data.args[0]);
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
}

static void handle_clone3(struct seccomp_notif_resp *resp)
{
    SEC_DBM("\t---clone3 syscall---");

    threadCount[domain]++;
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
}

static void handle_exit(struct seccomp_notif_resp *resp)
{
    SEC_DBM("\t----exit syscall----");

    threadCount[domain]--;
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
}


/* Handle notifications that arrive via the SECCOMP_RET_USER_NOTIF file
   descriptor, 'notifyFd'. */

static void handle_notifications()
{
    struct seccomp_notif        *req;
    struct seccomp_notif_resp   *resp;
    struct seccomp_notif_sizes  sizes;

    alloc_seccomp_notif_buffers(&req, &resp, &sizes);

    int             retval;
    struct Supervisor* sp = &supervisors[domain];

	atomic_fetch_sub(&counter, 1);
	supervisors[domain].fd = 0;
	threadCount[domain] = 0;
	while (!supervisors[domain].fd); 

    SEC_DBM("\t[S%d]: Handling notifications...", domain);

	struct pollfd fds[1] = {
        {
            .fd  = sp->fd,
            .events = POLLIN,
        },
    };

    /* Loop handling notifications */
    for (;;) {
		/* Watch stdin (supervisor's fd) to see when it has input. */
		/* Wait for next notification, returning info in '*req' */
		memset(req, 0, sizes.seccomp_notif);
        retval = poll(fds, 1, 0);
		if (retval == -1)
			perror("pselect()");
		else if (retval) {
			SEC_DBM("\t[S%d]: Got Call.", domain);

			if (ioctl(sp->fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
				if (errno == EINTR)
					continue;
				err(EXIT_FAILURE, "\t[S%d]: ioctl-SECCOMP_IOCTL_NOTIF_RECV", domain);
			}
		}
		else if (sp->status && threadCount[domain] == 1) {			
			break;
		}
		else {
			// timeout expired
			continue;
		}

		SEC_DBM("\t[S%d]: received notifaction id [%lld], from tid: %d, syscall nr: %d\n", 
			domain, req->id, req->pid, req->data.nr);

		if (!cookie_is_valid(sp->fd, req->id)) {
			perror("ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
			continue;
		}

		/* Prepopulate some fields of the response */
		resp->id = req->id;     /* Response includes notification ID */
		resp->flags = 0;
		resp->val = 0;

		// Handle specific syscalls
		switch(req->data.nr) {
			case __NR_pkey_mprotect:
				handle_pkey_mprotect(req, resp);
				break;
			case __NR_mmap:
				handle_mmap(req, resp);
				break;
			case __NR_munmap:
				handle_munmap(req, resp);
				break;
			case __NR_clone3:
				handle_clone3(resp);
				break;
			case __NR_exit:
				handle_exit(resp);
				break;
			default:
				break;
		}

		/* Send a response to the notification */

		SEC_DBM("\t[S%d]: sending response "
			"(flags = %#x; val = %lld; error = %d)",
			domain, resp->flags, resp->val, resp->error);

		if (ioctl(sp->fd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
			if (errno == ENOENT)
				SEC_DBM("\t[S%d]: response failed with ENOENT; "
					"perhaps target process's syscall was "
					"interrupted by a signal?\n", domain);
			else
				perror("ioctl-SECCOMP_IOCTL_NOTIF_SEND");
		}
		SEC_DBM("\t--------------------\n");
    }

    free(req);
    free(resp);
    SEC_DBM("\t[S%d]: terminating **********\n", domain);
}

/* Implementation of the supervisor thread:

   (1) obtains the notification file descriptor
   (2) handles notifications that arrive on that file descriptor. */

static void * supervisor(void *arg)
{   
    int* pDomain = (int*)arg;
    domain = *pDomain;

    SEC_DBM("\t[S%d]: up and running...", domain);

    while(1) {        
	handle_notifications();
    }

    return NULL;
}


void process_setup(const char *fifo_path, const char *ret_path)
{
	int fd, fd2;
	char buffer[BUFFER_SIZE];

	fd = open(fifo_path, O_RDONLY);
	if (fd == -1)
	{
		perror("Error opening Fifo");
		exit(EXIT_FAILURE);
	}

	ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE);
	if (bytes_read == -1)
	{
		perror("Error reading from Fifo");
		exit(EXIT_FAILURE);
	}
	close(fd);

	buffer[bytes_read] = '\0';
	fd2 = open(ret_path, O_WRONLY);
	if (fd2 == -1)
	{
		perror("Error opening Fifo");
		exit(EXIT_FAILURE);
	}

	dup2(fd2, STDOUT_FILENO);
	close(fd2);
	char *argv[] = {buffer, NULL};
	execv(argv[0], argv);

	perror("execv");
	exit(EXIT_FAILURE);
}


void init_process_pool()
{
	int i = 0;
	pid_t pid;
	for (i = 0; i < NUM_PROCESSES; ++i)
	{
		pid = fork();
		if (pid == 0)
		{
			char fifo_path[40], ret_path[40];
			snprintf(fifo_path, sizeof(fifo_path), "/tmp/fifo/fifo_%d", getpid());
			snprintf(ret_path, sizeof(ret_path), "/tmp/ret/ret_%d", getpid());

			int fifo_status = mkfifo(fifo_path, 0666);
			int ret_status = mkfifo(ret_path, 0666);

			if (fifo_status == -1 || ret_status == -1)
			{
				perror("Error creating fifo");
				exit(EXIT_FAILURE);
			}

			process_setup(fifo_path, ret_path);
			exit(EXIT_SUCCESS);
		}
		else
		{
			procIDs[i] = pid;
		}
	}
}


void *zombie_handler(void *arg)
{
	while (1)
	{
		pid_t pid;
		int status;
		while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
		{
			SEC_DBM("Reaped zombie process with PID %d\n", pid);
			char fifo_name[50], ret_name[50];

			snprintf(fifo_name, sizeof(fifo_name), "/tmp/fifo/fifo_%d", pid);
			snprintf(ret_name, sizeof(ret_name), "/tmp/ret/ret_%d", pid);

			// int unlink_fifo = unlink(fifo_name);
			// int unlink_ret = unlink(ret_name);

			// if (unlink_fifo == -1 || unlink_ret == -1)
			// {
			// 	perror("Error deleting FIFO");
			// 	exit(EXIT_FAILURE);
			// }

			int pipefd[2];
			if (pipe(pipefd) == -1)
			{
				perror("pipe failed");
				exit(EXIT_FAILURE);
			}

			pid_t new_pid = fork();
			if (new_pid == 0)
			{
				close(pipefd[0]);
				SEC_DBM("Child process with PID %d started\n", getpid());
				char fifo_path[50], ret_path[40];
				snprintf(fifo_path, sizeof(fifo_path), "/tmp/fifo/fifo_%d", getpid());
				snprintf(ret_path, sizeof(ret_path), "/tmp/ret/ret_%d", getpid());

				int fifo_status = mkfifo(fifo_path, 0666);
				int ret_status = mkfifo(ret_path, 0666);

				if (fifo_status == -1 || ret_status == -1)
				{
					perror("Error creating ret_path");
					exit(EXIT_FAILURE);
				}

				ssize_t bytes_written = write(pipefd[1], "ok", 2);

				if (bytes_written == -1)
				{
					perror("Error writing to Fifo");
					exit(EXIT_FAILURE);
				}

				process_setup(fifo_path, ret_path);
				close(pipefd[1]);
				exit(EXIT_SUCCESS);
			}
			else
			{
				char buffer[3];
				close(pipefd[1]);

				read(pipefd[0], buffer, sizeof(buffer));
				close(pipefd[0]);

				atomic_int expected;
				for (int i = 0; i < NUM_PROCESSES; i++)
				{
					expected = 0;
					int ret_val = __atomic_compare_exchange_n(&procIDs[i], &expected, new_pid, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
					if (ret_val != 0) break;
				}
			}
		}
	}
}

void *log_domains(void *arg) {
    long long microseconds_since_epoch;
    int non_zero_count;

    char* output_file = eager_mpk ? "/tmp/faastlane/domains.csv" : "/tmp/faastion/domains.csv";
    FILE *file = fopen(output_file, "w");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    while (1) {
        non_zero_count = atomic_load(&counter) + 14;

        struct timeval tv;
        gettimeofday(&tv, NULL);
		microseconds_since_epoch = (long long)(tv.tv_sec) * 1000000LL + (long long)(tv.tv_usec);

        fprintf(file, "%d,%lld\n", non_zero_count, microseconds_since_epoch);
        fflush(file);

        // Wait for 100 us
        usleep(100);
    }

    fclose(file);
    return NULL;
}

void *managed_supervisor()
{
    // Setting the buffers for request and response
    struct seccomp_notif *req;
    struct seccomp_notif_resp *resp;
    struct seccomp_notif_sizes sizes;

    alloc_seccomp_notif_buffers(&req, &resp, &sizes);

    while (1)
    {
            poll(fds, MAX_FDS, 0); // polling multiple fds

			for (int i = 0; i < MAX_FDS; ++i) {
        		if (fds[i].revents & POLLIN) {
            		fprintf(stderr,"File descriptor %d has data to read\n", fds[i].fd);
					memset(req, 0, sizes.seccomp_notif);
                // Accepting the seccomp unotify request
                if (ioctl(fds[i].fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1){
                    err(EXIT_FAILURE, "\tioctl-SECCOMP_IOCTL_NOTIF_RECV");
                }

                if (!cookie_is_valid(fds[i].fd, req->id)){
                        perror("ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
                        continue;
                }

                resp->id = req->id;
                resp->error = resp->val = 0;
                // Responding to the request with continue flag
                resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;

                // Sending the request to the respective fd
                if (ioctl(fds[i].fd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1){
                    perror("ioctl-SECCOMP_IOCTL_NOTIF_SEND");
                }
        	}
    	}
    }
    free(req);
    free(resp);
    return NULL;
}

void initialize_memory_isolation()
{   
	char* mpk_env = getenv("EAGER_MPK");
	if (mpk_env != NULL) {
		eager_mpk = atoi(mpk_env);
	}
	char* perms_env = getenv("EAGER_PERMS");
	if (perms_env != NULL) {
		eager_perms = atoi(perms_env);
	}

	if (eager_perms && eager_mpk) {
		fprintf(stderr, "Only one or neither mode should be selected!\n");
		exit(EXIT_FAILURE);
	}
	if (eager_perms) {
    	fprintf(stderr, "Executing in Eager permissions mode.\n");
	}
	else if (eager_mpk) {
    	fprintf(stderr, "Executing in Eager MPK mode.\n");
	}
	else {
    	fprintf(stderr, "Executing in Lazy permissions mode (default).\n");
	}

    /* Init arrays */
    init_cache(cache, NUM_DOMAINS);
    init_supervisors(supervisors, NUM_DOMAINS);
    init_thread_count(threadCount, NUM_DOMAINS);
	init_null_fd();
	
    pthread_t tid;
    if (pthread_create(&tid, NULL, zombie_handler, NULL) != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
    }

	pthread_t tid_s;
    if (pthread_create(&tid_s, NULL, managed_supervisor, NULL) != 0) {
		perror("pthread_create");
		exit(EXIT_FAILURE);
    }

    init_process_pool();

	pthread_sandbox_init();

    /* Start supervisors */
    pthread_t workers[NUM_DOMAINS];
    for (int i = 2; i < NUM_DOMAINS; i++) {
		int *pDomain = (int *)malloc(sizeof(int));
		*pDomain = i;
		pthread_create(&workers[i], NULL, supervisor, pDomain);
    }

	pthread_t thread_id;
    pthread_create(&thread_id, NULL, log_domains, NULL);
}
