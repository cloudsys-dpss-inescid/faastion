#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <link.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "../common/common.h"
#include "../erim/erim.h"

typedef struct {
    const char* lib_name;
    void* start_addr;
    size_t size;
} lib_info;

void init_erim();

extern int is_initialized;
