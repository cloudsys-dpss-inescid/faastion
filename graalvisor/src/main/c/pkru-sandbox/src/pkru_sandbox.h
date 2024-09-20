#ifndef PKRU_SANDBOX_H
#define PKRU_SANDBOX_H

#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
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


/**
 * @brief Request structure for the worker thread.
 */
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

/**
 * @brief Structure representing a Worker of a domain.
 */
typedef struct worker {
    pthread_t thread;         /** Worker thread for the domain. */
    pthread_cond_t cond;      /** Condition variable for signaling new requests. */
    pthread_mutex_t lock;     /** Lock for synchronizing access to the conditional variable. */
} worker_t;

/**
 * @brief Structure representing a Monitor of a domain.
 */
typedef struct monitor {
    pthread_t thread;         /** Monitor thread for supervising the domain. */
    int seccomp_fd;           /** Seccomp file descriptor. */
} monitor_t;

/**
 * @brief Function to protect memory regions associated with a given library.
 * 
 * @param library The name of the library.
 * @param pkey The protection key to apply.
 */
void protect_library(const char* library, int pkey);

/**
 * @brief Cleanup resources and exit the program.
 */
void cleanup_and_exit(void);

/**
 * @brief Install a seccomp filter and return the seccomp file descriptor.
 * 
 * @return int The seccomp file descriptor.
 */
int install_seccomp_filter(void);

/**
 * @brief Handle system calls for a given protection key.
 * 
 * @param pkey The protection key to handle.
 */
void handle_syscalls(int pkey);

/**
 * @brief Monitor thread function to supervise system calls for a domain.
 * 
 * @param arg Pointer to the domain's protection key.
 */
void* monitor(void* arg);

/**
 * @brief Execute a function within a specific domain (sandboxed).
 * 
 * @param domain The domain number to execute within.
 * @param ret Pointer to the function's return value.
 * @param ret_size Pointer to the size of the return value.
 * @param fun The function to be executed.
 * @param arg The argument passed to the function.
 * @param arg_size The size of the argument passed.
 * @return int Status code of the execution.
 */
int pkru_sandbox_call(int domain, void** ret, size_t* ret_size, void (*fun)(int), void *arg[], int argc);

/**
 * @brief Worker thread function to execute domain tasks.
 * 
 * @param arg Pointer to the domain's protection key.
 */
void* worker(void* arg);

/**
 * @brief Wrapper function for launching worker threads.
 * 
 * @param arg Pointer to the domain's protection key.
 */
void* worker_wrapper(void* arg);

/**
 * @brief Initialize the PKRU sandbox and its associated domains, workers, and monitors.
 * 
 * @return int 0 on success, -1 on error.
 */
int pkru_sandbox_init();

pthread_cond_t *get_domain_cond(int pkey);
pthread_mutex_t *get_domain_lock(int pkey);

#endif // PKRU_SANDBOX_H
