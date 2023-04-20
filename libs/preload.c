#define _GNU_SOURCE

#include <stdarg.h>
#include <stdio.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <link.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>


typedef struct {
    const char* lib_name;
    void* start_addr;
    size_t size;
} lib_info;

//static void* (*real_pthread_create) (pthread_t *, const pthread_attr_t *, void *(*)(void *), void *) = NULL;
static void* (*real_realloc)        (void *, size_t) = NULL;
static void* (*real_malloc)         (size_t) = NULL;
static void* (*real_dlopen)         (const char *, int) = NULL;
static void* (*real_mmap)           (void *, size_t, int, int, int, off_t) = NULL;

static void __attribute__((constructor)) init(void)
{
    //real_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
    real_realloc        = dlsym (RTLD_NEXT, "realloc");
    real_malloc         = dlsym (RTLD_NEXT, "malloc");
    real_dlopen         = dlsym (RTLD_NEXT, "dlopen");
    real_mmap           = dlsym (RTLD_NEXT, "mmap");
}

int callback(struct dl_phdr_info *info, size_t size, void *data) {
    lib_info* callback_data = (lib_info*)data;

    if (!strcmp(info->dlpi_name, callback_data->lib_name)) {
        unsigned long total_size = 0;

        for(int i = 0; i < (int) info->dlpi_phnum; i++) {
            unsigned long size = (unsigned long)info->dlpi_phdr[i].p_memsz;
            total_size += size;
        }

        callback_data->start_addr = (void *)info->dlpi_addr;
        callback_data->size = total_size;
        return 1;
    }

    return 0;
}


void *dlopen(const char *filename, int flag) 
{   
    void *handle = real_dlopen(filename, flag);

    // get address and size of library
    lib_info info = { filename, NULL, -1};
    dl_iterate_phdr(callback, &info);
    void *start_addr = info.start_addr;
    size_t size = info.size;

    fprintf(stderr, "dlopen():\n\tLibrary: %s\n\tLoad start address: %p\n\tSize: %lu bytes\n", filename, start_addr, size);
    return handle;
}

void *malloc(size_t size) 
{
    void *result = real_malloc(size);
    fprintf(stderr, "malloc(): %ld bytes\n", size);
    return result;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) 
{
    void *result = real_mmap(addr, length, prot, flags, fd, offset);
    fprintf(stderr, "mmap():\n\tAddress: %p\n\tLength:%ld\n", addr, length);
    return result;
}

void *realloc(void *ptr, size_t size) 
{
    void *result = real_realloc(ptr, size);
    fprintf(stderr, "realloc(): %ld bytes\n", size);
    return result;
}

//int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg) {
//    int result = real_pthread_create(thread, attr, start_routine, arg);
//    fprintf(stderr, "pthread_create(): thread with id %lu\n", *thread);
//    return result; 
//}
