#define _GNU_SOURCE

#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include "preload.h"

AppMap appMap;
ThreadMap threadMap;

/* Function pointers declarations */
static int( * real_pthread_create)(pthread_t * , const pthread_attr_t * , void * ( * )(void * ), void * ) = NULL;
static void( * real_pthread_exit)(void *) = NULL;
static void * ( * real_dlopen)(const char * , int) = NULL;

/* Constructor */
static void __attribute__((constructor)) init(void) {
    initAppMap(&appMap);
    initThreadMap(&threadMap);

    // Initialize isolation
    if (erim_init(32768, ERIM_FLAG_ISOLATE_TRUSTED | ERIM_FLAG_SWAP_STACK, 16)) {
        exit(EXIT_FAILURE);
    }
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

int findEmptyDomain() {
    unsigned long index = -1;

    for (int i = 1; i < 16; i++) {
        index = hash_int(i);
        if (threadMap.buckets[index] == NULL)
            return i;
    }
    // Domain not found
    return -1;
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

    PRELOAD_DBM("Saving library addresses and sizes...");
    getMemoryRegions(&appMap, id, pathname);

    printAppMap(appMap, 1);

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
        PRELOAD_DBM("New thread running on domain %d in map", domain);
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
    PRELOAD_DBM("Removing thread running on domain %d from map", domain);
    removeThread(&threadMap, domain, currentThread);

    real_pthread_exit(value_ptr);
}
#endif
