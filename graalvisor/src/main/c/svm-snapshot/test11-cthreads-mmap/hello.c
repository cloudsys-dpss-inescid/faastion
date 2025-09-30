#include <stdio.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include "../graal_isolate.h"
#include <sys/mman.h>
#include <errno.h>

pthread_t worker = 0;
void* buffer = NULL;

void* run_function(void* args) {
    for (int i = 0; i < 1024; i++) {
        if (buffer == NULL) {
            buffer = mmap(NULL, 1024, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
            if (buffer == MAP_FAILED) {
                fprintf(stderr, "[background thread] mmap error %d\n", errno);
            }
        } else {
            int ret = munmap(buffer, 1024);
            if (ret) {
                fprintf(stderr, "[background thread] munmap error %d\n", errno);
            }
            buffer = NULL;

        }
        fprintf(stderr, "[background thread] buffer = %p\n", buffer);
        sleep(1);
    }
}

int graal_create_isolate (graal_create_isolate_params_t* params, graal_isolate_t** isolate, graal_isolatethread_t** thread) {
    *thread = NULL;
    *isolate = NULL;
    return 0;
}

int graal_tear_down_isolate(graal_isolatethread_t* thread) {
    return 0;
}

void entrypoint(graal_isolatethread_t* thread, const char* fin, const char* fout, unsigned long fout_len) {
    int tid = syscall(__NR_gettid);
    int pid = getpid();
    if (worker == 0) {
        // Note: this test is not easy to reproduce. You will have to delay the checkpoint and
        // check that the syscall has been blocked.
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
