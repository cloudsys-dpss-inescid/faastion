#define _GNU_SOURCE
#include <dlfcn.h>
#include <poll.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#include "core/pkru_sandbox.h"
#include "core/memory_map.h"

void copy_file(const char* sourcePath, const char* destPath) {
    FILE *sourceFile = fopen(sourcePath, "rb");
    FILE *destFile = fopen(destPath, "wb");

    if (sourceFile == NULL || destFile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceFile)) > 0) {
        fwrite(buffer, 1, bytesRead, destFile);
    }

    fclose(sourceFile);
    fclose(destFile);
}

int main()
{
    // Initialize phtread sandboxes and allocate pkeys.
    if (pkru_sandbox_init()) {
        fprintf(stderr, "failed to initialize pthread sandboxes\n");
        cleanup_and_exit();
    }

    // Note: in faastion, we would do this step before we enter native code.
    // Load dynamic library into a new namespace.
    void *handle = dlmopen(LM_ID_NEWLM, "./libapp.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    
    // int domain = book_available_domain(gettid());
    // if (domain == 0) {
    //    fprintf(stderr, "error: failed to book available domain for thread %d\n", gettid());
    //    exit(1);
    // }
    int pkey = 2;
    copy_file("/proc/self/smaps", "smaps_before");
    protect_memory_regions(pkey);
    copy_file("/proc/self/smaps", "smaps_after");

    // Call the target function.
    void (*fun)(void*, size_t, void**, size_t*) = (void (*)(void*, size_t, void**, size_t*))dlsym(handle, "fun");
    if (!fun) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        exit(EXIT_FAILURE);
    }
    
    // Enter the sandbox, call the function, leave the sandbox.
    void* ret = NULL;
    size_t ret_size = 0;
    pkru_sandbox_call(pkey, &ret, &ret_size, fun, "Hello?", strlen("Hello?") + 1);

    printf("Function returned %s (size = %lu)\n", (char*)ret, ret_size);

    dlclose(handle);

    return EXIT_SUCCESS;
}
