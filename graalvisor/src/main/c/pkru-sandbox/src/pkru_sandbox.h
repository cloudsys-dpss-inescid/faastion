#ifndef PKRU_SANDBOX_H
#define PKRU_SANDBOX_H

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <jni.h>
#include "domain_manager.h"


// Domain IDs from 0 to 15.
#define DOMAINS 16
#define DEFAULT_DOMAIN 0
#define LOADER_DOMAIN 1
#define MAX_ARGS 8

// TODO - have a table for constant conversion.
// Domain to PKRU conversion table.
#define DOMAIN_TO_PKRU(domain) (\
    (domain == 0) ? 0x0 : \
    (domain == 1) ? 0x55555551 : \
    (domain == 2) ? 0x55555545 : \
    (domain == 3) ? 0x55555515 : \
    (domain == 4) ? 0x55555455 : \
    (domain == 5) ? 0x55555155 : \
    (domain == 6) ? 0x55554555 : \
    (domain == 7) ? 0x55551555 : \
    (domain == 8) ? 0x55545555 : \
    (domain == 9) ? 0x55515555 : \
    (domain == 10) ? 0x55455555 : \
    (domain == 11) ? 0x55155555 : \
    (domain == 12) ? 0x54555555 : \
    (domain == 13) ? 0x51555555 : \
    (domain == 14) ? 0x45555555 : \
    (domain == 15) ? 0x15555555 : \
    -1 \
)

#ifndef __wrpkru
#define __wrpkru(PKRU_ARG)			    \
  do {									\
    asm volatile ("xor %%ecx, %%ecx\n\txor %%edx, %%edx\n\tmov %0,%%eax\n\t.byte 0x0f,0x01,0xef\n\t" \
	      : : "n" (PKRU_ARG)					\
	      :"eax", "ecx", "edx");			\
  } while (0)
#endif

#define __wrpkrumem(PKRU_ARG)			    \
  do {									\
    asm volatile ("xor %%ecx, %%ecx\n\txor %%edx, %%edx\n\tmov %0,%%eax\n\t.byte 0x0f,0x01,0xef\n\t" \
	      : : "m" (PKRU_ARG)					\
	      :"eax", "ecx", "edx");			\
  } while (0)

#ifndef __rdpkru
#define __rdpkru()                              \
  ({                                            \
    unsigned int eax, edx;                      \
    unsigned int ecx = 0;                       \
    unsigned int pkru;                          \
    asm volatile(".byte 0x0f,0x01,0xee\n\t"     \
                 : "=a" (eax), "=d" (edx)       \
                 : "c" (ecx));                  \
    pkru = eax;                                 \
    pkru;                                       \
  })
#endif

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
