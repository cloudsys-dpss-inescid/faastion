#ifndef __MEMISOLATION_H__
#define __MEMISOLATION_H__

#define _GNU_SOURCE
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "utils/appmap.h"
#include "utils/threadmap.h"
#include "helpers/helpers.h"

/* erim includes */
#include <common.h>
#include <erim.h>


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
void insert_app_id(int domain, const char* id);
char* get_app_id(int domain);
int find_app_domain(const char* id);

/* Thread synchronization */
void lock();
void unlock();

/* MPK */
void set_permissions(const char* id, int protectionFlag, int pkey);

/* Seccomp */
void install_notify_filter(int domain);

/* Domain management */
int find_empty_domain();

#endif
