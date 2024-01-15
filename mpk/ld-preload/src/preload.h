#ifndef __MEMISOLATION_H__
#define __MEMISOLATION_H__

#define _GNU_SOURCE
#include <dlfcn.h>

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
  #define SEC_DBG(...)
#endif

/* Lazy loading */
char* get_app_id(int domain);
void insert_app_id(int domain, const char* id);

/* Thread synchronization */
void lock();
void unlock();

/* Supervisors */
void wait_set(int domain);
void signal_set(int domain);
void wait_filter(int domain);
void signal_filter(int domain);
void wait_perms(int domain);
void signal_perms(int domain);
void wait_handler(int domain);
void signal_handler(int domain);
void update_supervisor_app(int domain, const char* app);
void update_supervisor_status(int domain);

/* Seccomp */
void install_notify_filter(int domain);

/* Domain management */
int find_domain(const char* id);

#endif
