#include <algorithm>
#include <cstring>
#include <cstdio>
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

#include "preload.h"

#define MALLOC
#define REALLOC
#define FREE
#define MMAP
#define MUNMAP
#define DLOPEN
#define PTHREAD_CREATE
#define PTHREAD_EXIT

std::unordered_map<std::string, std::vector<MemoryRegion>> apps;
std::unordered_map<int, std::vector<pthread_t>> runningThreads;
std::mutex runningThreadsMutex;

static __thread int no_hook = 0;

int verbose = 1;  // Set this flag to 1 for logging, or 0 to disable logging

/* Function pointers declarations */
static int( * real_pthread_create)(pthread_t * , const pthread_attr_t * , void * ( * )(void * ), void * ) = NULL;
static int( * real_munmap)(void *, size_t) = NULL;
static void( * real_pthread_exit)(void *) = NULL;
static void( * real_free)(void *) = NULL;
static void * ( * real_malloc)(size_t) = NULL;
static void * ( * real_realloc)(void *, size_t) = NULL;
static void * ( * real_mmap)(void *, size_t, int, int, int, off_t) = NULL;
static void * ( * real_dlopen)(const char * , int) = NULL;


/* Constructor */
void __attribute__((constructor)) init() {
    // Initialize isolation
    if (erim_init(32768, ERIM_FLAG_ISOLATE_TRUSTED)) {
        exit(EXIT_FAILURE);
    }
    erim_switch_to_untrusted;
}



/* Auxiliary functions */
void logMessage(const char* message) {
    if (verbose) {
        pthread_t tid = pthread_self();  // Get the thread ID
        fprintf(stderr, "[%lu] %s\n", (unsigned long)tid, message);
    }
}

void setApplicationPermissions(const char* appID, int protectionFlag, int pkey) {
    auto it = apps.find(appID);
    if (it == apps.end()) {
        errExit("Application ID not found in the memory map.");
    }

    const std::vector<MemoryRegion>& memoryRegions = it->second;
    for (const MemoryRegion& region : memoryRegions) {
        if (pkey_mprotect(region.address, region.size, protectionFlag, pkey) == -1) {
            errExit("pkey_mprotect error");
        }
    }
}

std::string extractBaseName(const std::string& filePath) {
    size_t lastSlashPos = filePath.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        return filePath.substr(lastSlashPos + 1);
    }
    return filePath;
}

int isDomainEmpty() {
    if (runningThreads[1].empty()) return 1;
    return 0;
}

void getMemoryRegions(const char * appID, const char * path) {
    std::string libraryName = extractBaseName(path);    

    std::ifstream mapsFile("/proc/self/maps");
    if (!mapsFile) {
        errExit("Failed to open /proc/self/maps");
    }

    std::string line;
    line.reserve(256);
    MemoryRegion memoryRegion;
    while (std::getline(mapsFile, line)) {
        if (line.find(libraryName) == std::string::npos) {
            continue;
        }

        unsigned long startAddress, endAddress;
        sscanf(line.c_str(), "%lx-%lx", &startAddress, &endAddress);

        memoryRegion.address = reinterpret_cast<void*>(startAddress);
        memoryRegion.size = endAddress - startAddress;

        apps[appID].push_back(memoryRegion);
    }

    mapsFile.close();
}

void printApps() {
    for (const auto& entry : apps) {
        const std::string& appID = entry.first;
        const std::vector<MemoryRegion>& memoryRegions = entry.second;

        std::cout << "App ID: " << appID << std::endl;

        for (const MemoryRegion& region : memoryRegions) {
            std::cout << "\tStart Address: " << region.address << ", Size: " << region.size << " bytes" << std::endl;
        }

        std::cout << std::endl;
    }
}


/* Memory allocation and mapping */
#ifdef MALLOC
void * malloc(size_t size) {
    void *ret;

    if (real_malloc == NULL) {
        real_malloc = reinterpret_cast < decltype(real_malloc) > (dlsym(RTLD_NEXT, "malloc"));
    }

    if (no_hook) {
        return (*real_malloc)(size);
    }

    no_hook = 1;
    ret = (*erim_malloc)(size);
    no_hook = 0;

    return ret;
}
#endif

#ifdef FREE
void free(void * ptr) {
    if (real_free == NULL) {
        real_free = reinterpret_cast < decltype(real_free) > (dlsym(RTLD_NEXT, "free"));
    }

    if (no_hook) {
        real_free(ptr);
        return;
    }

    no_hook = 1;
    erim_free(ptr);
    no_hook = 0;
}
#endif

#ifdef REALLOC
void * realloc(void * ptr, size_t size) {
    void *ret;

    if (real_realloc == NULL) {
        real_realloc = reinterpret_cast < decltype(real_realloc) > (dlsym(RTLD_NEXT, "realloc"));
    }

    if (no_hook) {
        return (*real_realloc)(ptr, size);
    }

    no_hook = 1;
    ret = (*erim_realloc)(ptr, size);
    no_hook = 0;

    return ret;
}
#endif

#ifdef MMAP
void * mmap(void * addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void *ret;

    if (real_mmap == NULL) {
        real_mmap = reinterpret_cast < decltype(real_mmap) > (dlsym(RTLD_NEXT, "mmap"));
    }

    if (no_hook) {
        return (*real_mmap)(addr, length, prot, flags, fd, offset);
    }

    no_hook = 1;
    ret = erim_mmap_domain(addr, length, prot, flags, fd, offset, ERIM_EXEC_DOMAIN(__rdpkru()));
    no_hook = 0;

    return ret;
}
#endif

#ifdef MUNMAP
int munmap(void * addr, size_t length) {
    int ret;

    if (real_munmap == NULL) {
        real_munmap = reinterpret_cast < decltype(real_munmap) > (dlsym(RTLD_NEXT, "munmap"));
    }

    if (no_hook) {
        return real_munmap(addr, length);
    }

    no_hook = 1;
    ret = erim_munmap(addr, length);
    no_hook = 0;

    return ret;
}
#endif

/* Library loading */

#ifdef DLOPEN
void * dlopen(const char * input, int flag) {
    if (real_dlopen == NULL) {
        real_dlopen = reinterpret_cast < decltype(real_dlopen) > (dlsym(RTLD_NEXT, "dlopen"));
    }
    
    if (!input || std::strchr(input, ':') == nullptr) {
        return real_dlopen(input, flag);
    }

    // Parse input
    std::string concat = extractBaseName(input);    
    char appID[256];

    sscanf(concat.c_str(), "%[^:]", appID);

    // remove "lib" prefix
    std::string application_id(appID);
    application_id.erase(0, 3);

    std::string pathname(input);
    size_t pos = pathname.find(application_id);
    pathname.erase(pos, application_id.length() + 1);

    void * handle = real_dlopen(pathname.c_str(), flag | RTLD_GLOBAL);

    logMessage("Saving library addresses and sizes...");
    getMemoryRegions(application_id.c_str(), pathname.c_str());
    printApps();
    
    std::remove(input);
    return handle;
}
#endif

/* Threads */

#ifdef PTHREAD_CREATE
int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * ( * start_routine)(void * ), void * arg) {
    if (real_pthread_create == NULL) {
        real_pthread_create = reinterpret_cast < decltype(real_pthread_create) > (dlsym(RTLD_NEXT, "pthread_create"));
    }

    int result = real_pthread_create(thread, attr, start_routine, arg);

    if (result == 0) {
        int domain = ERIM_EXEC_DOMAIN(__rdpkru());
        {
            std::lock_guard<std::mutex> lock(runningThreadsMutex);
            runningThreads[domain].push_back(*thread);
        }
    }

    return result;
}
#endif

#ifdef PTHREAD_EXIT
void pthread_exit(void* value_ptr) {
    if (real_pthread_exit == NULL) {
        real_pthread_exit = reinterpret_cast < decltype(real_pthread_exit) > (dlsym(RTLD_NEXT, "pthread_exit"));
    }
    
    pthread_t currentThread = pthread_self();
    int domain = ERIM_EXEC_DOMAIN(__rdpkru());

    {
        std::lock_guard<std::mutex> lock(runningThreadsMutex);
        std::vector<pthread_t> tvec = runningThreads[domain];
        tvec.erase(std::remove(tvec.begin(), tvec.end(), currentThread), tvec.end());
    }

    real_pthread_exit(value_ptr);
}
#endif
