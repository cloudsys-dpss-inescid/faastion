#ifndef __MEMISOLATION_H__
#define __MEMISOLATION_H__

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdatomic.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include "utils/mansupervisor.h"

/*
 * Debug prints
 */
#ifdef SEC_DBG
  #define SEC_DBM(...)				\
    do {					\
      fprintf(stderr, __VA_ARGS__);		\
      fprintf(stderr, "\n");			\
    } while(0)
#else // disable debug
  #define SEC_DBM(...)
#endif

#define NUM_PROCESSES 20

extern __thread int domain;
extern volatile atomic_int procIDs[NUM_PROCESSES];

/* Supervisors */
void wait_sem();
void signal_sem();
void reset_env(const char* application, int isLast);

/* Domain management */
void acquire_domain(const char* app, int* fd);
void find_domain_eager(const char* app);

/* Initalizer */
void initialize_memory_isolation();

/* Auxiliary */
void insert_memory_regions(char* id, const char* path, pthread_mutex_t mutex);

#endif
