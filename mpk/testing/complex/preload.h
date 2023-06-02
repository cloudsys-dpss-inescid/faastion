#ifndef PRELOAD_H
#define PRELOAD_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <algorithm>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <link.h>
#include <mutex>
#include <pthread.h>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "../common/common.h"
#include "../erim/erim.h"

#define errExit(msg) do { \
    std::cerr << msg << std::endl; \
    exit(EXIT_FAILURE); \
} while (0)


struct LibraryInfo {
    const char* appID;
    const char* path;
};

struct MemoryRegion {
    void* address;
    size_t size;
};

void setApplicationPermissions(const char* appID, int protectionFlag);

extern std::unordered_map<int, std::vector<pthread_t>> runningThreads;

#endif // PRELOAD_H
