/*
 * test_application.c
 *
 */

#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Erim includes
#include <common.h>
#include <erim.h>

static __thread char* regular = NULL;

void protectMemoryRegions() {
    FILE* mapsFile = fopen("/proc/self/maps", "r");
    if (!mapsFile) {
        fprintf(stderr, "Failed to open /proc/self/maps\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), mapsFile)) {
        if (strstr(line, "libinc.so") == NULL) {
            continue;
        }

        unsigned long startAddress, endAddress;
        sscanf(line, "%lx-%lx", &startAddress, &endAddress);

        void * address = (void*)startAddress;
        size_t size = endAddress - startAddress;

        pkey_mprotect(address, size, PROT_READ|PROT_WRITE|PROT_EXEC, 1);
    }

    fclose(mapsFile);
}

int wrapper(int a) {
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
    
    protectMemoryRegions();
    
    __wrpkru(ERIM_DOMAIN(1));
    ret = inc(a);
    __wrpkru(ERIM_DOMAIN(0));

    dlclose(handle);

    return ret;
}

int main(int argc, char **argv) {
    int a = 321;

    // trusted (regular) domain -> 0 (can access both domains 0 and 1, pkru = 0x55555550)
    // untrusted (isolated) domain -> 1 (con only access domain 1, pkry = 0x55555553)
    if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK, 2)) {
        exit(EXIT_FAILURE);
    }

    ERIM_SWITCH_STACK(ERIM_DOMAIN_STACK_LOC(1), regular);
    a = wrapper(a);
    ERIM_SWITCH_BACK(regular);
    fprintf(stderr, "a = %d\n", a);
    return 0;
}
