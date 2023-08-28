#ifndef __SECCOMP_H__
#define __SECCOMP_H__

#include <unistd.h>

char *get_pathname(char name[]);
int exec(char pathname[], char *argv[], int mode);
int get_seccomp_mode(char *mode);

#endif