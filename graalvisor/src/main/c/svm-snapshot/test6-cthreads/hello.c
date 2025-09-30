#include <stdio.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include "../graal_isolate.h"
#include <stdlib.h>

pthread_t worker = 0;

void* run_function(void* args) {
    int myvar = 0;
    int tid = syscall(__NR_gettid);
    int pid = getpid();
    printf("calling malloc\n");
    void* useless = malloc(50);
    while(myvar < 50) {
        register void *sp asm ("sp");
        fprintf(stderr, "[background thread] sp = %p myvar = %d tid = %d pid = %d\n", sp, myvar++, tid, pid);
        for (int i = 0; i < 100000000; i++) ;
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
    printf("start of application\n");
    int tid = syscall(__NR_gettid);
    int pid = getpid();
    fprintf(stderr, "[foreground thread] tid = %d pid = %d\n", tid, pid);
    if (worker == 0) {
        pthread_create(&worker, NULL, run_function, NULL);
    }
    sleep(1); // Give some time for the worker to do something.
}

int graal_detach_thread(graal_isolatethread_t* thread) {
    return 0;
}

int graal_attach_thread(graal_isolate_t* isolate, graal_isolatethread_t** thread) {
    *thread = NULL;
    return 0;
}
