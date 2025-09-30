#ifndef CR_MALLOC
#define CR_MALLOC

#include "malloc.h"
#include "malloc_internal.h"
#include <sys/types.h>  /* for pid_t */

#define MAX_MSPACE 1024

// TODO - add the other memory allocated functions.
mspace get_mspace_mapping();
int get_mspace_count();

mspace get_mspace(unsigned int pkey);
void *get_mspace_lock(unsigned int pkey);
unsigned int get_locked_thread(unsigned int pkey);

#endif
