#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>

static void* (*real_malloc)(size_t) = NULL;
static void* (*real_realloc)(void*, size_t) = NULL;
static void* (*real_mmap)(void*, size_t, int, int, int, off_t) = NULL;

static void init(void) {
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_realloc = dlsym(RTLD_NEXT, "realloc");
    real_mmap = dlsym(RTLD_NEXT, "mmap");
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    if (real_mmap == NULL) {
        init();
    }
    void *result = real_mmap(addr, length, prot, flags, fd, offset);
    fprintf(stderr, "mmap: %ld\n", length);
    return result;
}

void *malloc(size_t size) {
    if (real_malloc == NULL) {
        init();
    }
    void *result = real_malloc(size);
    fprintf(stderr, "malloc: %ld\n", size);
    return result;
}

void *realloc(void *ptr, size_t size) {
    if (real_realloc == NULL) {
        init();
    }
    void *result = real_realloc(ptr, size);
    fprintf(stderr, "realloc: %ld\n", size);
    return result;
}
