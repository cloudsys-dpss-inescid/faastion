#ifndef PRELOAD_H
#define PRELOAD_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dlfcn.h>
#include "../erim/common.h"
#include "../erim/erim.h"

#define errExit(msg) do { \
    fprintf(stderr, "%s\n", msg); \
    exit(EXIT_FAILURE); \
} while (0)

struct MemoryRegion {
    void* address;
    size_t size;
};

void setApplicationPermissions(const char* appID, int protectionFlag, int pkey);
int isDomainEmpty();

#endif // PRELOAD_H
