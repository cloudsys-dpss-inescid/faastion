#define _GNU_SOURCE

#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include "preload.h"

AppMap appMap;
ThreadMap threadMap;
char* appIds[16];
pthread_mutex_t mutex;

/* Function pointers declarations */
static void * ( * real_dlopen)(const char * , int) = NULL;

/* Constructor */
static void __attribute__((constructor)) init(void) {
    initAppMap(&appMap);
    initThreadMap(&threadMap);
    initAppArray(&appIds);
    pthread_mutex_init(&mutex, NULL);
    
    // Initialize isolation
    if (erim_init(32768, ERIM_FLAG_ISOLATE_TRUSTED | ERIM_FLAG_SWAP_STACK, 16)) {
        exit(EXIT_FAILURE);
    }
}

/* App functions */
void insertApp(int domain, const char* id) {
    appIds[domain] = strdup(id);
}

int findApp(const char* id) {
    for (int i = 0; i < 16; i++) {
        if (appIds[i] != NULL && !strcmp(id, appIds[i])) {
            return i;
        }
    }
    return -1; // App not found
}

char* getApp(const char* domain) {
    return (appIds[domain] != NULL) ? appIds[domain] : "";
}

/* Thread Map functions */
void insertThreadInMap(int domain) {
    insertThread(&threadMap, domain, pthread_self());
}

int isEmpty(int domain) {
    return threadMap.buckets[index] == NULL;
}

int findEmptyDomain() {
    for (int i = 1; i < 16; i++) {
        if (isEmpty(i))
            return i;
    }
    return -1; // Empty Domain not found
}

void joinThreads(int domain) {
    ThreadNode* currentNode = threadMap->buckets[domain];

    while (currentNode != NULL) {
        pthread_join(currentNode->threadId, NULL);
        currentNode = currentNode->next;
        removeThread(&threadMap, domain, currentNode->threadId);
    }
}

/* */
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
