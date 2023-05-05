#include "preload.h"

int is_initialized = 0;

/* Function pointers declarations */
static int (*real_pthread_create)   (pthread_t *, const pthread_attr_t *, void *(*)(void *), void *) = NULL;
static void* (*real_dlopen)         (const char *, int) = NULL;


/* Constructor */
static void __attribute__((constructor)) init(void)
{
    real_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
    real_dlopen         = dlsym (RTLD_NEXT, "dlopen");
}


/* Auxiliary functions */
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

void init_erim() {
    // init isolation and sh mem
    if(erim_init(8192, ERIM_FLAG_ISOLATE_TRUSTED | ERIM_FLAG_INTEGRITY_ONLY)) {
        exit(EXIT_FAILURE);
    }
    // scanmem for wrpkru
    if(erim_memScan(NULL, NULL, ERIM_UNTRUSTED_PKRU)) {
        exit(EXIT_FAILURE);
    }
}


/* Memory allocation and mapping */
void *malloc(size_t size) 
{   
    return erim_malloc(size);
}

void *zalloc(size_t size) 
{   
    return erim_zalloc(size);
}

void *realloc(void *ptr, size_t size) 
{
    return erim_realloc(ptr, size);
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) 
{
    return erim_mmap_isolated(addr, length, prot, flags, fd, offset);
}

void free(void* ptr) {
    erim_free(ptr);
}

int munmap(void* addr, size_t length) {
    return erim_munmap(addr, length);
}


/* Library loading */
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


/* Threads */
int pthread_create(pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)(void*), void* arg) {
    //fprintf(stderr, "pthread_create(): thread with id %lu\n", *thread);
    return real_pthread_create(thread, attr, start_routine, arg);
}
