#define _GNU_SOURCE

#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include "preload.h"

AppMap appMap;
ThreadMap threadMap;
static __thread int no_hook = 0;
int verbose = 0;  // Set this flag to 1 for logging, or 0 to disable logging

/* Function pointers declarations */
static int( * real_pthread_create)(pthread_t * , const pthread_attr_t * , void * ( * )(void * ), void * ) = NULL;
static void( * real_pthread_exit)(void *) = NULL;
static void * ( * real_dlopen)(const char * , int) = NULL;
static void * ( * real_mmap)(void *, size_t, int, int, int, off_t) = NULL;
static int( * real_munmap)(void *, size_t) = NULL;
static void * ( * real_malloc)(size_t) = NULL;
static void * ( * real_realloc)(void *, size_t) = NULL;
static void( * real_free)(void *) = NULL;

/* Constructor */
static void __attribute__((constructor)) init(void) {
    initAppMap(&appMap);
    initThreadMap(&threadMap);
    // Initialize isolation
    if (erim_init(32768, ERIM_FLAG_ISOLATE_TRUSTED)) {
        exit(EXIT_FAILURE);
    }
    erim_switch_to_untrusted;
}

/* App functions */
int isEmpty(int domain) {
    unsigned long index = hash_int(domain);
    ThreadNode* currentNode = threadMap.buckets[index];
    while (currentNode != NULL) {
        if (currentNode->domain == domain) {
            return 0;
        }
        currentNode = currentNode->next;
    }
    // Domain not found
    return 1;
}

void setAppPermissions(const char* id, int protectionFlag, int pkey) {
    size_t count;
    MemoryRegion* regions = getRegions(appMap, (char*)id, &count);

    for (size_t i = 0; i < count; ++i) {
        if (pkey_mprotect(regions[i].address, regions[i].size, protectionFlag, pkey) == -1) {
            fprintf(stderr, "pkey_mprotect error\n");
            exit(EXIT_FAILURE);
        }
    }
}

/* Memory allocation and mapping */
#ifdef MALLOC
void * malloc(size_t size) {
    void *ret;

    if (real_malloc == NULL) {
        real_malloc =  (void *(*) (size_t)) dlsym(RTLD_NEXT, "malloc");
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
        real_free = (void (*) (void *)) dlsym(RTLD_NEXT, "free");
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
        real_realloc = (void *(*) (void *, size_t)) dlsym(RTLD_NEXT, "realloc");
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
        real_mmap = (void *(*) (void *, size_t, int, int, int, off_t)) dlsym(RTLD_NEXT, "mmap");
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
        real_munmap = (int (*) (void *, size_t)) dlsym(RTLD_NEXT, "munmap");
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
        real_dlopen = (void *(*) (const char *, int)) dlsym(RTLD_NEXT, "dlopen");
    }
    
    if (!input || strchr(input, ':') == NULL) {
        return real_dlopen(input, flag);
    }

    // Parse input
    char pathname[256] = "";
    char libname[256] = "lib";
    char id[256] = "";
    char filename[256] = "";
    
    char * basename = extractBaseName(input);
    sscanf(basename, "%[^:]:%s", id, filename);
    strcat(libname, filename);

    size_t size = strlen(input) - strlen(basename);
    strncpy(pathname, input, size);
    strcat(pathname, libname);

    void *handle = real_dlopen(pathname, flag | RTLD_GLOBAL);

    logMessage("Saving library addresses and sizes...", verbose);
    getMemoryRegions(&appMap, id, pathname);

    printAppMap(appMap, verbose);

    remove(input);
    return handle;
}
#endif

/* Threads */

#ifdef PTHREAD_CREATE
int pthread_create(pthread_t * thread, const pthread_attr_t * attr, void * ( * start_routine)(void * ), void * arg) {
    if (real_pthread_create == NULL) {
        real_pthread_create = (int (*) (pthread_t *, const pthread_attr_t *, void * ( * start_routine)(void * ), void *)) dlsym(RTLD_NEXT, "pthread_create");
    }

    int result = real_pthread_create(thread, attr, start_routine, arg);

    if (result == 0) {
        int domain = ERIM_EXEC_DOMAIN(__rdpkru());
        logMessage("New thread in map", verbose);
        insertThread(&threadMap, domain, *thread);
    }

    return result;
}
#endif

#ifdef PTHREAD_EXIT
void pthread_exit(void* value_ptr) {
    if (real_pthread_exit == NULL) {
        real_pthread_exit =  (void (*) (void *)) dlsym(RTLD_NEXT, "pthread_exit");
    }
    
    pthread_t currentThread = pthread_self();
    int domain = ERIM_EXEC_DOMAIN(__rdpkru());
    logMessage("Removing thread from map", verbose);
    removeThread(&threadMap, domain, currentThread);

    real_pthread_exit(value_ptr);
}
#endif
