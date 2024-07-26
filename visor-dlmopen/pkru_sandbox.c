#define _GNU_SOURCE

#include "pkru_sandbox.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/audit.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <poll.h>
#include <errno.h>
#include <pthread.h>

// Request to execute by the worker domain thread.
typedef struct request {
    // Function to invoke.
    void (*fun)(void*, size_t, void**, size_t*);
    // // Argument to use in the function call.
    void * arg;
    // Size (in bytes) of the argument.
    size_t arg_size;
    // Address where the returned is present.
    void* ret;
    // Size (in bytes) of the return value.
    size_t ret_size;
} request_t;

// There is a single worker thread per domain.
typedef struct worker {
    // Worker thread pointer (may be used for joining the thread).
    pthread_t thread;
    // Conditional variable used to signal a new request.
    pthread_cond_t cond;
    // Lock used to synchronize access to the conditional variable.
    pthread_mutex_t lock;
} worker_t;

// Arenas to exchange data between domains.
static void* arenas;
// Saves the tid currently using each domain.
static pid_t* thread_domain;
// Seccomp thread that intercepts system calls.
static pthread_t seccomp_thread;
// Threads that will be running functions inside domains.
static worker_t worker_threads[DOMAINS];
// Seccomp fd to be used by the seccomp thread.
static int seccomp_fd = 0;

#define ARENA(domain) ((void*) (((char*)arenas) + domain * getpagesize()))


static void protect_library(const char* library, int pkey)
{
    FILE* mapsFile = fopen("/proc/self/maps", "r");
    if (!mapsFile) {
        fprintf(stderr, "Failed to open /proc/self/maps\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), mapsFile)) {
        if (strstr(line, library) == NULL) {
            continue;
        }

        unsigned long start, finish;
        char r, w, x;

        sscanf(line, "%lx-%lx %c%c%c",
            &start, &finish, &r, &w, &x);

        int prot_flags = 0;
        if (r == 'r') prot_flags |= PROT_READ;
        if (w == 'w') prot_flags |= PROT_WRITE;
        if (x == 'x') prot_flags |= PROT_EXEC;

        void * address = (void*)start;
        size_t size = finish - start;

        pkey_mprotect(address, size, prot_flags, pkey);
        fprintf(stderr, "Moving to domain %d: %s", pkey, line);
    }

    fclose(mapsFile);
}

int install_seccomp_filter()
{
    struct sock_filter filter[] = {
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, arch))),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, nr))),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_mmap, 0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),

        // default rule
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("error: failed to prctl(NO_NEW_PRIVS)");
        return -1;
    }

    int fd = syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (fd < 0) {
        perror("error: failed to seccomp(SECCOMP_SET_MODE_FILTER)");
        return -1;
    }

    return fd;
}

void handle_syscalls(int fd)
{
    struct seccomp_notif_sizes sizes;
    if (syscall(SYS_seccomp, SECCOMP_GET_NOTIF_SIZES, 0, &sizes) < 0) {
        fprintf(stderr, "error: failed to seccomp(SECCOMP_GET_NOTIF_SIZES)");
        return;
    }

    struct seccomp_notif *req = (struct seccomp_notif*)malloc(sizes.seccomp_notif);
    struct seccomp_notif_resp *resp = (struct seccomp_notif_resp*)malloc(sizes.seccomp_notif_resp);
    struct pollfd fds[1] = {
        {
            .fd  = fd,
            .events = POLLIN,
        },
    };

    for(;;) {

        // Wait for a notification
        int events = poll(fds, 1, 0);
        if (events < 0) {
            perror("error: failed to pool for events");
            continue;
        } else if (fds[0].revents & POLLNVAL) {
            break;
        } else if (events > 1) {
            perror("warning: received multiple events at once!\n");
        }

        // Receive notification
        memset(req, 0, sizes.seccomp_notif);
        memset(resp, 0, sizes.seccomp_notif_resp);
        if (ioctl(fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_RECV)");
            continue;
        }

        // Validate notification
        if (ioctl(fd, SECCOMP_IOCTL_NOTIF_ID_VALID, &req->id) == -1 ) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
            continue;
        }

        // Send response
        resp->id = req->id;
        long long unsigned int *args = req->data.args;
        switch (req->data.nr) {
            // TODO - we also need to track clone to make sure we track additional threads that also perform syscalls.
            case __NR_mmap:
                resp->val = syscall(__NR_mmap, args[0], args[1], args[2], args[3], args[4], args[5]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                int domain = get_thread_domain(req->pid);
                fprintf(stderr, "thread id %d domain %d mmap: memory %p size %lu!\n",
                    req->pid, domain, (void*) resp->val, (size_t) args[1]);
                if (domain != 0) {
                    if(pkey_mprotect((void*) resp->val, (size_t) args[1], (int) args[2], domain) == -1) {
                        fprintf(stderr, "error: failed to mprotect %p for %lu bytes\n", (void*) resp->val, (size_t) args[1]);
                    }
                }
                break;
            default:
                fprintf(stderr, "warning: unhandled syscall %d!\n", req->data.nr);
                resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
                break;
        }

        if (ioctl(fd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_SEND)");
            continue;
        }
    }

    close(fd);
    free(req);
    free(resp);
}

void* monitor(void* arg)
{
    while (seccomp_fd == 0) ;
    handle_syscalls(seccomp_fd);
    return NULL;
}

void* worker(void* arg)
{
    int domain = (int) ((long) arg);
    pthread_mutex_t* lock = &(worker_threads[domain].lock);
    pthread_cond_t* cond = &(worker_threads[domain].cond);
    pthread_mutex_init(lock, NULL);
    pthread_cond_init(cond, NULL);

    for (;;) {
        pthread_mutex_lock(lock);
        pthread_cond_wait(cond, lock);
        pthread_mutex_unlock(lock);
        // TODO - jump to target domain.
        // TODO - call user function
        // TODO - go back to domain zero.
        pthread_cond_broadcast(cond);
    }
}

int pkru_sandbox_init(void)
{
	arenas = mmap(0, DOMAINS * getpagesize(), PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
    if (arenas == MAP_FAILED) {
        fprintf(stderr, "error: failed to mmap arenas\n");
        return -1;
    }
    memset(arenas, 0, DOMAINS * getpagesize());

    for (int i = 1; i < DOMAINS; i++) {
        if(syscall(SYS_pkey_alloc, 0, 0) < 0) {
            fprintf(stderr, "error: failed to allocate pkey %d\n", i);
            return -1;
        }
        if (pkey_mprotect(ARENA(i), getpagesize(), PROT_READ|PROT_WRITE, i)) {
            fprintf(stderr, "error: failed protect arena at %p with pkey %d\n", ARENA(i), i);
            return -1;
	    }
        fprintf(stdout, "Protected arena at %p - %p with pkey %d\n", ARENA(i), ((char*)ARENA(i) + getpagesize()), i);
    }

    thread_domain = (pid_t*) malloc(DOMAINS * sizeof(pid_t));
    if (thread_domain == NULL) {
        fprintf(stderr, "error: failed to malloc memory for the thread domain table\n");
        return -1;
    }
    memset(thread_domain, 0, DOMAINS * sizeof(pid_t));

    // Move ld (ld-linux-x86-64.so.2) to domain 1 so that it can be shared.
    protect_library("ld-linux-x86-64", LOADER_DOMAIN);

    // Launch the monitor thread.
    pthread_create(&seccomp_thread, NULL, monitor, &seccomp_fd);

    // Install seccomp filter to monitor memory-related operations.
    seccomp_fd = install_seccomp_filter();
    if (seccomp_fd < 0) {
        fprintf(stderr, "error: failed install seccomp filter\n");
        return -1;
    }

    // We start from domain 2 since domain 1 is reserved for the loader library.
    for (int i = 2; i < DOMAINS; i++) {
        // Setting the thread domain is needed so that the monitor knowns which domain to use.
        set_thread_domain(gettid(), i);
        // Launching the worker thread.
        //pthread_create(&worker_threads[i], NULL, worker, (void*) i);
    }
    set_thread_domain(gettid(), 0);


	return 0;
}

// TODO - this function will be deleted and replaced by the worker function.
void* trampoline(void* arg)
{
    int* domain = (int*) arg;
    request_t* request = (request_t*) ARENA(*domain);

    // Adding thread to domain.
    set_thread_domain(gettid(), *domain);

    // Changing domain and calling the function.
    __wrpkrumem(DOMAIN_TO_PKRU(*domain) & DOMAIN_TO_PKRU(LOADER_DOMAIN));
    request->fun(request->arg, request->arg_size, &(request->ret), &(request->ret_size));
    __wrpkru(DEFAULT_DOMAIN);
    return NULL;
}

int pkru_sandbox_call(int domain, void** ret, size_t* ret_size, void (*fun)(void*, size_t, void**, size_t*), void* arg, size_t arg_size)
{
    pthread_t trampolier;

    // The domain arena is setup in the following way:
    // |--- request (sizeof(request_t bytes) long) ---|--- arg (arg_size bytes long) ---|--- ret (ret_size bytes long) ---|
    request_t* request = (request_t*) ARENA(domain);
    request->arg = (void*) ((char*)ARENA(domain) + sizeof(request_t));
    request->arg_size = arg_size;
    request->fun = fun;

    // Copying the function call argument to arena.
    memcpy(request->arg, arg, arg_size);

    // Creating and waiting for worker thread. // TODO - we should have pre-created this thread.
    pthread_create(&trampolier, NULL, trampoline, &domain); // TODO - add thread to list of threads in domain.
    pthread_join(trampolier, NULL);

    // Copying return value to arena and setting ret and ret_size pointers.
    *ret = (void*) (((char*) request->arg) + arg_size);
    *ret_size = request->ret_size;
    memcpy(*ret, request->ret, request->ret_size);
	return 0;
}

// TODO - make it thread safe.
int get_thread_domain(pid_t tid)
{
    for (int i = 0; i < DOMAINS; i++) {
        if (thread_domain[i] == tid) {
            return i;
        }
    }
    return 0;
}

// TODO - make it thread safe.
void set_thread_domain(pid_t tid, int domain)
{
    thread_domain[domain] = tid; // TODO - this will overwrite the previous one...
}

// TODO - make it thread safe.
int book_available_domain(pid_t tid)
{
    for (int i = 3; i < DOMAINS; i++) {
        if (thread_domain[i] == 0) {
            thread_domain[i] = tid;
            return i;
        }
    }
    return 0;
}