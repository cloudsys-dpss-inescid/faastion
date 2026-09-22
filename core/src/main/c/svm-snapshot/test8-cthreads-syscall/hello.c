#include <stdio.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <asm/prctl.h>
#include "../graal_isolate.h"

pthread_t worker = 0;
int myvar = 0;

void* run_function(void* args) {
    while(myvar < 10) {
        void* tls = NULL;

        // Read tid and pid.
        int tid = syscall(__NR_gettid);
        int pid = getpid();

        // Read tls pointer.
        if (syscall(SYS_arch_prctl, ARCH_GET_FS, &tls)) {
            fprintf(stderr, "failed to get tls\n");
        }

        // Read stack pointer.
        register void *sp asm ("sp");

        fprintf(stderr, "[background thread] sp = %p myvar = %d tls = %p tid = %d pid = %d\n", sp, myvar++, tls, tid, pid);
        sleep(1);
    }
}

int graal_create_isolate (graal_create_isolate_params_t* params, graal_isolate_t** isolate, graal_isolatethread_t** thread) {
    *thread = NULL;
    *isolate = NULL;
    return 0;
}

int graal_tear_down_isolate(graal_isolatethread_t* thread) {
    pthread_join(worker, NULL);
}

void entrypoint(graal_isolatethread_t* thread, const char* fin, const char* fout, unsigned long fout_len) {
    int tid = syscall(__NR_gettid);
    int pid = getpid();
    fprintf(stderr, "[foreground thread] tid = %d pid = %d\n", tid, pid);
    if (worker == 0) {
        pthread_create(&worker, NULL, run_function, NULL);
    }
    sleep(2); // Give the worker the change to print something.
}

int graal_detach_thread(graal_isolatethread_t* thread) {
    return 0;
}

int graal_attach_thread(graal_isolate_t* isolate, graal_isolatethread_t** thread) {
    *thread = NULL;
    return 0;
}
