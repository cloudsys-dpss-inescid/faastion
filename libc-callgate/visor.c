#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <unistd.h>

// TODO - how do we attest that memory sen't into libc belongs to a particular app?
#define APP_DOMAIN   0x3

static void print_file(char* filepath, char* logpath)
{
    FILE* logfile = fopen(logpath, "w");
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", filepath);
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        fprintf(logfile, "%s", line);
    }

    fclose(logfile);
    fclose(file);
}

static void protect_page(const void* ptr, int pkey, int prot)
{
    size_t page_size = sysconf(_SC_PAGESIZE);
    void* page = (void*) ((((size_t)ptr) / page_size) * page_size);

    pkey_mprotect(page, page_size, prot, pkey);
    fprintf(stderr, "Moving to domain %d: %p\n", pkey, page);
}

static void protect_library(const char* library, int pkey)
{
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

        unsigned long start, finish;
        char r, w, x;

        sscanf(line, "%lx-%lx %c%c%c",
            &start, &finish, &r, &w, &x);

        int prot_flags = 0;
        if (r == 'r') prot_flags |= PROT_READ;
        if (w == 'w') prot_flags |= PROT_WRITE;
        if (x == 'x') prot_flags |= PROT_EXEC;

        void * address = (void*)start;
        size_t size = finish - start;

        pkey_mprotect(address, size, prot_flags, pkey);
        fprintf(stderr, "Moving to domain %d: %s", pkey, line);
    }

    fclose(mapsFile);
}

void print_tls() {
    #define ARCH_GET_FS   0x1003
    void* tls = NULL;

    if (syscall(SYS_arch_prctl, ARCH_GET_FS, &tls)) {
        fprintf(stderr, "failed to read tls\n");
        return;
    }
    fprintf(stderr, "TLS = %p Pthread_self = %p errno = %p\n", tls, pthread_self(), __errno_location);
}

int main()
{
    // Initialize phtread sandboxes and allocate pkeys.
    if (pthread_sandbox_init()) {
        fprintf(stderr, "failed to initialize pthread sandboxes\n");
    }

    void *handle = dlopen("./libmmap.so", RTLD_NOW);
    if (!handle) {
        fprintf(stderr, "dlopen error: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    //print_file("/proc/self/maps", "maps_after_dlopen");
    //print_file("/proc/self/smaps", "smaps_after_dlopen");

    void* (*fun)() = (void* (*)())dlsym(handle, "fun");
    if (!fun) {
        fprintf(stderr, "dlsym error: %s\n", dlerror());
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    // Tagging app into a separate domain.
    protect_library("libmmap.so", APP_DOMAIN);

    // Tagging global variable in a global domain. // TODO - should be domain 1?
    protect_page(&stderr, APP_DOMAIN, PROT_READ);

    // Entre the sandbox, call the function, leave the sandbox.
    void* ret = NULL;
    pthread_sandbox_call(APP_DOMAIN, &ret, fun, "Hello?");

    dlclose(handle);
    return EXIT_SUCCESS;
}
