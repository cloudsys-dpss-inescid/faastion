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

        pkey_mprotect(address, size, PROT_READ|PROT_WRITE, 1);
    }

    fclose(mapsFile);
}

int wrapper(int a) {
    int ret = 123;

    void *handle = dlopen("./libinc.so", RTLD_LAZY);
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
    
    __wrpkru(ERIM_UNTRUSTED_PKRU);
    ret = inc(a);
    __wrpkru(ERIM_TRUSTED_PKRU);

    dlclose(handle);

    return ret;
}

int main(int argc, char **argv) {
    int a = 321;

    // trusted (regular) domain -> 0 (can access both domains 0 and 1, pkru = 0x55555550)
    // untrusted (isolated) domain -> 1 (con only access domain 1, pkry = 0x55555553)
    if(erim_init(8192, ERIM_FLAG_ISOLATE_UNTRUSTED | ERIM_FLAG_SWAP_STACK)) {
        exit(EXIT_FAILURE);
    }

    ERIM_SWITCH_TO_ISOLATED_STACK;
    a = wrapper(a);
    ERIM_SWITCH_TO_REGULAR_STACK;
    fprintf(stderr, "a = %d\n", a);
    return 0;
}
