#ifndef PKRU_SANDBOX_H
#define PKRU_SANDBOX_H

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <jni.h>
#include "domain_manager.h"
#include "pkru.h"


#define MAX_ARGS 8

typedef struct request {
    void (*fun)(int);             /** Function pointer for the request. */
    void *args[MAX_ARGS];         /** Arguments for the function. */
    size_t arg_sizes[MAX_ARGS];   /** Sizes of the arguments. */
    int num_args;                 /** Number of arguments. */
    void *ret;                    /** Return value address. */
    size_t ret_size;              /** Size of the return value. */
    JNIEnv *env;
    jobject obj;
} request_t;

typedef struct worker {
    pthread_t thread;         /** Worker thread for the domain. */
    sem_t request;
    sem_t response;
    pthread_mutex_t request_lock;
} worker_t;

typedef struct monitor {
    pthread_t thread;         /** Monitor thread for supervising the domain. */
    int seccomp_fd;           /** Seccomp file descriptor. */
} monitor_t;


int pkru_sandbox_init();
void cleanup_and_exit(void);

void *worker(void* arg);
void *worker_wrapper(void *arg);

pthread_mutex_t *get_request_lock(int pkey);
void notify_worker(int domain);
void wait_worker(int domain);

void print_systime();

void *jni_monitor(void* arg);
void *jvm_monitor(void* arg);
int install_jni_filter();
int install_jvm_filter();

extern worker_t worker_threads[DOMAINS];
extern monitor_t monitor_threads[DOMAINS];

#endif // PKRU_SANDBOX_H
