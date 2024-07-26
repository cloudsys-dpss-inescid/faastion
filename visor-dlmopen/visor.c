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
#include "pkru_sandbox.h"

int main()
{
    // Initialize phtread sandboxes and allocate pkeys.
    if (pkru_sandbox_init()) {
        fprintf(stderr, "failed to initialize pthread sandboxes\n");
    }

    // Note: in faastion, we would delay this step until we decide to enter native code.
    // Instead of booking a domain, faastion would keep track of which mmaps were performed
    // on behalf of each application.
    int domain = book_available_domain(gettid());
    if (domain == 0) {
        fprintf(stderr, "error: failed to book available domain for thread %d\n", gettid());
    }

    // Load dynamic library into a new namespace.
    void *handle = dlmopen(LM_ID_NEWLM, "./libapp.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    // Call the target function.
    void (*fun)(void*, size_t, void**, size_t*) = (void (*)(void*, size_t, void**, size_t*))dlsym(handle, "fun");
    if (!fun) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Entre the sandbox, call the function, leave the sandbox.
    void* ret = NULL;
    size_t ret_size = 0;
    pkru_sandbox_call(domain, &ret, &ret_size, fun, "Hello?", strlen("Hello?") + 1);

    printf("Function returned %s (size = %lu)\n", (char*)ret, ret_size);

    dlclose(handle);
    return EXIT_SUCCESS;
}
