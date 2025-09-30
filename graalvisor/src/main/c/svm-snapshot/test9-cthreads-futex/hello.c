#include <stdio.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include <asm/prctl.h>
#include "../graal_isolate.h"
#include <time.h>

pthread_t worker = 0;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void* run_function(void* args) {
    for (;;) {
        // Read tid and pid.
        int tid = syscall(__NR_gettid);
        int pid = getpid();

        pthread_mutex_lock(&lock);
        fprintf(stderr, "[background thread] before wait tid = %d pid = %d\n", tid, pid);
        pthread_cond_wait(&cond, &lock);
        fprintf(stderr, "[background thread] after wait tid = %d pid = %d\n", tid, pid);
        pthread_mutex_unlock(&lock);
        sleep(1); // Give time for the main thread to reach the cond wait.
        pthread_cond_broadcast(&cond);
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
    sleep(1); // Give time for the worker to reach the cond wait.
    pthread_cond_broadcast(&cond);
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);
}

int graal_detach_thread(graal_isolatethread_t* thread) {
    return 0;
}

int graal_attach_thread(graal_isolate_t* isolate, graal_isolatethread_t** thread) {
    *thread = NULL;
    return 0;
}
