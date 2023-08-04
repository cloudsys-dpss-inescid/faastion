/*
 * test_application.c
 *
 */

#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Erim includes
#include <common.h>
#include <erim.h>

static __thread char* regular = NULL;

void protectMemoryRegions(const char * library, int pkey) {
    FILE* mapsFile = fopen("/proc/self/maps", "r");
    if (!mapsFile) {
        fprintf(stderr, "Failed to open /proc/self/maps\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), mapsFile)) {
        if (strstr(line, library) == NULL) {
            continue;
        }

        unsigned long startAddress, endAddress;
        sscanf(line, "%lx-%lx", &startAddress, &endAddress);

        void * address = (void*)startAddress;
        size_t size = endAddress - startAddress;

        pkey_mprotect(address, size, PROT_READ|PROT_WRITE|PROT_EXEC, pkey);
    }

    fclose(mapsFile);
}

int incWrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libinc.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return -1;
    }  

    int (*inc)(int) = (int (*)(int))dlsym(handle, "inc");
    if (!inc) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return -1;
    }
    
    protectMemoryRegions("libinc.so", 1);
    
    __wrpkru(ERIM_DOMAIN_1);
    ret = inc(a);
    __wrpkru(ERIM_MONITOR);

    dlclose(handle);

    return ret;
}

void* incMain(void* arg) {
    int value = *(int*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_1_STACK_LOC, regular);
    value = incWrapper(value);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "value = %d\n", value);

    return NULL;
}

void logWrapper(const char * message) {
    void *handle = dlopen("./liblog.so", RTLD_NOW | RTLD_DEEPBIND);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        return;
    }  

    void (*logMessage)(const char*) = (void (*)(const char*))dlsym(handle, "logMessage");
    if (!logMessage) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        return;
    }
    
    protectMemoryRegions("liblog.so", 2);
    
    __wrpkru(ERIM_DOMAIN_2);
    logMessage(message);
    __wrpkru(ERIM_MONITOR);
    
    dlclose(handle);
}

void* logMain(void* arg) {
    const char* message = (const char*)arg;
    ERIM_SWITCH_STACK(ERIM_DOMAIN_2_STACK_LOC, regular);

    memcpy(ERIM_DOMAIN_2_STACK_LOC, message, strlen(message) + 1);

    char *charPtr = (char*)ERIM_DOMAIN_2_STACK_LOC;
    char cpMessage[strlen(message) + 1];
    strcpy(cpMessage, charPtr);
    logWrapper(cpMessage);

    ERIM_SWITCH_BACK(regular);

    return NULL;
}

void run_threads() {   
    pthread_t * workers = (pthread_t*) malloc(sizeof(pthread_t)*2);
    const char* message = "Hello, Threads!";
    int a = 321;

    if (pthread_create(&workers[0], NULL, incMain, (void*)&a) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    if (pthread_create(&workers[1], NULL, logMain, (void*)message) != 0){
        perror("Can't create thread\n");
        exit(1);
    }
    
    for(int i = 0; i < 2; i++) {
        if(pthread_join(workers[i], NULL)) {
            perror("Thread can't join\n");
            exit(1);
        }
    }
    free(workers);
}

int main(int argc, char **argv) {
    if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK, 3)) {
        exit(EXIT_FAILURE);
    }

    run_threads();

    return 0;
}
