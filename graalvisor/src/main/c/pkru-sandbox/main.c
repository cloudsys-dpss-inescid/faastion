#define _GNU_SOURCE

#include "pkru_sandbox.h"
#include "memory_map.h"
#include "hash_table.h"
#include "seccomp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <syscall.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>
#include <dlfcn.h>
#include <sched.h>
#include <linux/sched.h>
#include <malloc.h>
#include <time.h>
#include <sys/wait.h>
#include <link.h>



// Threads that will be running functions inside domains.
worker_t worker_threads[DOMAINS];

// Threads that will be supervising domains.
monitor_t monitor_threads[DOMAINS];

static FILE *latency_breakdown_file;

#ifdef PRINT_TIMER
void print_systime() {
    static char timestamp[18];
    static struct timespec tnow = {0,};
    clock_gettime(CLOCK_MONOTONIC, &tnow);
    long systime = tnow.tv_sec * 1.0e9 + tnow.tv_nsec;
    int bytes = snprintf(timestamp, 18, "%ld\n", systime);
    fwrite(timestamp, sizeof(char), bytes, latency_breakdown_file);
}
#else
void print_systime() {}
#endif

typedef void (*dl_init_t)(int, char **, char **);

static void (*original_run_constructor)(dl_init_t, int, char **, char **) = NULL;

void run_constructor(dl_init_t constructor, int argc, char **argv, char **env) {
    unsigned int pkey;
    unsigned int pkru;
    unsigned int unprivileged_domain;
    void (*fn)(dl_init_t, int, char **, char **);

    if (original_run_constructor == NULL) {
        original_run_constructor = dlsym(RTLD_NEXT, "run_constructor");
    }

    pkru = __rdpkru();
    pkey = worker_domain();
    fn = original_run_constructor;
    char **vargv = {NULL};
    char **venv = {NULL};
    if (pkey && is_running_untrusted()) {
        argc = 0;
        argv = vargv;
        env = venv;
        unprivileged_domain = DOMAIN_TO_PKRU(pkey) & DOMAIN_TO_PKRU(LOADER_DOMAIN);
        __wrpkrumem(unprivileged_domain);
    }
    fn(constructor, argc, argv, env);
    __wrpkrumem(pkru);
}

Elf64_Addr run_fixup(void *l, unsigned int reloc_arg) {
    Elf64_Addr retval;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;
    
    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554; 
    
    __wrpkru(DEFAULT_DOMAIN);
    retval = _dl_fixup(l, reloc_arg);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

pthread_mutex_t *get_request_lock(int domain) {
    return &(worker_threads[domain].request_lock);
}

static void protect_library(const char* library, int pkey)
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
        // fprintf(stdout, "Moving %p (%ld) to domain %d: %s", address, size, pkey, line);
    }

    fclose(mapsFile);
}

// FIXME: kernel resets pkru to 0x55555554 during signal handling
void handler(int signo)
{
    cleanup_and_exit();
}

void cleanup_and_exit()
{
    // cleanup_domains();
    // TODO - Kill workers
    // TODO - Kill monitors
    exit(0);
}

int pkru_sandbox_init()
{
    // Register the SIGINT handler
    // if (signal(SIGINT, handler) == SIG_ERR) {
    //     fprintf(stderr, "error: Unable to catch SIGINT\n");
    //     return -1;
    // }

    proc_tbl = new_hash_table(4096);

    start_active_waiting_count();

#ifdef PRINT_TIMER
    latency_breakdown_file = fopen("latency_breakdown.txt", "w");
    if (latency_breakdown_file == NULL) {
        fprintf(stderr, "error: failed to create latency breakdown file\n");
        return -1;
    }
#endif

    // Get domains ready for populating
    if (initialize_all_domains()) {
        fprintf(stderr, "error: failed initializing domains\n");
        return -1;
    }

    // Move ld (ld-linux-x86-64.so.2) to domain 1 so that it can be shared.
    protect_library("ld-linux-x86-64", LOADER_DOMAIN);

    seccomp_init();

    // Launch worker threads.
    for (int i = 1; i < DOMAINS; i++) {
        if (pthread_create(&(monitor_threads[i].thread), NULL, jni_monitor, (void*)(intptr_t) i) != 0) {
            fprintf(stderr, "error: failed creating monitor thread for domain %d\n", i);
            return -1;
        }

        if (pthread_create(&(worker_threads[i].thread), NULL, worker_wrapper, (void*)(intptr_t) i) != 0) {
            fprintf(stderr, "error: failed creating worker thread for domain %d\n", i);
            return -1;
        }
    }

    return 0;
}
