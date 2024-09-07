#define _GNU_SOURCE

#include "pkru_sandbox.h"
#include "domain_manager.h"
#include "memory_map.h"
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
#include <sys/types.h>
#include <signal.h>


// Threads that will be running functions inside domains.
static worker_t worker_threads[DOMAINS];

// Threads that will be supervising domains.
static monitor_t monitor_threads[DOMAINS];


void protect_library(const char* library, int pkey)
{
    FILE* mapsFile = fopen("/proc/self/maps", "r");
    if (!mapsFile) {
        fprintf(stderr, "error: failed to open /proc/self/maps\n");
        cleanup_and_exit();
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
        fprintf(stdout, "Moving %p (%ld) to domain %d: %s", address, size, pkey, line);
    }

    fclose(mapsFile);
}

void handler(int signo)
{
    cleanup_and_exit();
}

void cleanup_and_exit()
{
    cleanup_domains();
    free_memory_region_list();
    // TODO - Kill workers
    // TODO - Kill monitors
    exit(1);
}

int install_seccomp_filter()
{
    struct sock_filter filter[] = {
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, arch))),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, nr))),
        
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_munmap, 1, 0),        
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
        fprintf(stderr, "error: failed to prctl(NO_NEW_PRIVS)");
        cleanup_and_exit();
    }

    int fd = syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (fd < 0) {
        fprintf(stderr, "error: failed to seccomp(SECCOMP_SET_MODE_FILTER)");
        cleanup_and_exit();
    }

    return fd;
}

void handle_syscalls(int pkey)
{
    struct seccomp_notif_sizes sizes;
    if (syscall(SYS_seccomp, SECCOMP_GET_NOTIF_SIZES, 0, &sizes) < 0) {
        fprintf(stderr, "error: failed to seccomp(SECCOMP_GET_NOTIF_SIZES)");
        return;
    }

    struct seccomp_notif *req = (struct seccomp_notif*)malloc(sizes.seccomp_notif);
    struct seccomp_notif_resp *resp = (struct seccomp_notif_resp*)malloc(sizes.seccomp_notif_resp);
    
    int fd = monitor_threads[pkey].seccomp_fd;
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
                if (errno)
                    break;
                if (pkey != 0) {
                    fprintf(stdout, "thread id %d domain %d mmap: memory %p size %lu!\n",
                        req->pid, pkey, (void*) resp->val, (size_t) args[1]);
                    if (pkey_mprotect((void*) resp->val, (size_t) args[1], (int) args[2], pkey) == -1) {
                        fprintf(stderr, "error: failed to mprotect %p for %lu bytes\n", (void*) resp->val, (size_t) args[1]);
                    }
                } else {
                    fprintf(stdout, "Saving region: address %p size %lu prot %d! \n",(void*) resp->val, (size_t) args[1], (int) args[2]);
                    append_memory_region_node((void*) resp->val, (size_t) args[1], (int) args[2]);
                }
                
                break;
            case __NR_munmap:
                resp->val = syscall(__NR_munmap, args[0], args[1], args[2], args[3], args[4], args[5]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                if (errno)
                    break;
                if (pkey == 0) {
                    fprintf(stdout, "Deleting region: address %p size %lu\n",(void *)(args[0]), (size_t)(args[1]));
                    delete_memory_region_node((void *)(args[0]), (size_t)(args[1]));
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
    int pkey = (int) ((long) arg);

    // Wait until seccomp_fd is set
    while (monitor_threads[pkey].seccomp_fd == 0) ;

    handle_syscalls(pkey);
    return NULL;
}

int pkru_sandbox_call(int domain, void** ret, size_t* ret_size, void (*fun)(void*, size_t, void**, size_t*), void* arg, size_t arg_size)
{
    // The domain arena is setup in the following way:
    // |--- request (sizeof(request_t bytes) long) ---|--- arg (arg_size bytes long) ---|--- ret (ret_size bytes long) ---|
    request_t* request = (request_t*) get_domain_arena(domain);
    request->arg = (void*) ((char*)request + sizeof(request_t));
    request->arg_size = arg_size;
    request->fun = fun;

    // Copying the function call argument to arena.
    memcpy(request->arg, arg, arg_size);
     
    pthread_mutex_t* lock = &(worker_threads[domain].lock);
    pthread_cond_t* cond = &(worker_threads[domain].cond);
    
    pthread_cond_broadcast(cond);
    pthread_mutex_lock(lock);
    pthread_cond_wait(cond, lock);
    pthread_mutex_unlock(lock);
    fprintf(stderr, "User thread notify\n");
   
    // Copying return value to arena and setting ret and ret_size pointers.
    *ret = (void*) (((char*) request->arg) + arg_size);
    *ret_size = request->ret_size;
    memcpy(*ret, request->ret, request->ret_size);
	return 0;
}

void* worker(void* arg)
{
    int pkey = (int) ((long) arg);
    fprintf(stderr, "Worker for domain %d is running...\n", pkey);
    
    pthread_mutex_t* lock = &(worker_threads[pkey].lock);
    pthread_cond_t* cond = &(worker_threads[pkey].cond);
    pthread_mutex_init(lock, NULL);
    pthread_cond_init(cond, NULL);

    for (;;) {
        pthread_mutex_lock(lock);
        pthread_cond_wait(cond, lock);
        pthread_mutex_unlock(lock);
        fprintf(stderr, "Worker thread for domain %d notify\n", pkey);
        request_t* request = (request_t*) get_domain_arena(pkey);

        // Changing domain and calling the function.
        __wrpkrumem(DOMAIN_TO_PKRU(pkey) & DOMAIN_TO_PKRU(LOADER_DOMAIN));
        request->fun(request->arg, request->arg_size, &(request->ret), &(request->ret_size));
        __wrpkru(DEFAULT_DOMAIN);

        pthread_cond_broadcast(cond);
    }
    return NULL;
}

void* worker_wrapper(void* arg)
{
    int pkey = (int) ((long) arg);

    monitor_threads[pkey].seccomp_fd = install_seccomp_filter();
    if (pthread_create(&(worker_threads[pkey].thread), NULL, worker, (void*)(intptr_t)pkey)) {
        fprintf(stderr, "Error creating worker thread for domain %d\n", pkey);
        cleanup_and_exit();
    }
    return NULL;
}

int pkru_sandbox_init()
{
    // Register the SIGINT handler
    if (signal(SIGINT, handler) == SIG_ERR) {
        fprintf(stderr, "error: Unable to catch SIGINT\n");
        return -1;
    }

    // Get domains ready for populating
    if (initialize_all_domains()) {
        fprintf(stderr, "error: failed initializing domains\n");
        return -1;
    }

    // Move ld (ld-linux-x86-64.so.2) to domain 1 so that it can be shared.
    protect_library("ld-linux-x86-64", LOADER_DOMAIN);

    // Launch monitor threads.
    for (int i = 0; i < DOMAINS; i++) {
        if (pthread_create(&(monitor_threads[i].thread), NULL, monitor, (void*)(intptr_t) i) != 0) {
            fprintf(stderr, "error: failed creating monitor thread for domain %d\n", i);
            return -1;
        }
    }

    // Launch worker threads.
    for (int i = 1; i < DOMAINS; i++) {
        if (pthread_create(&(worker_threads[i].thread), NULL, worker_wrapper, (void*)(intptr_t) i) != 0) {
            fprintf(stderr, "error: failed creating worker_wrapper thread for domain %d\n", i);
            return -1;
        }
    }

    monitor_threads[DEFAULT_DOMAIN].seccomp_fd = install_seccomp_filter();
    return 0;
}
