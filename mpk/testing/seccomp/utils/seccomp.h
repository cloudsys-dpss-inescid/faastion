#ifndef __SECCOMP_H__
#define __SECCOMP_H__

#include <unistd.h>

/*
 * Debug prints
 */
#ifdef SECC_DBG
  #define SECC_DBM(...)				\
    do {					\
      fprintf(stderr, __VA_ARGS__);		\
      fprintf(stderr, "\n");			\
    } while(0)
#else // disable debug
  #define SECC_DBG(...)
#endif

char *get_pathname(char name[]);
int exec(char pathname[], char *argv[], int mode);
int get_seccomp_mode(char *mode);

#endif