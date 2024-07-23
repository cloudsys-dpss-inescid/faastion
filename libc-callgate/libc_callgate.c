/**
 * LibC Callgate.
 * This file is used to add hand-written libc function wrappers (variadic functions in particular).
 * Other functions will be handled through automatically generated wrappers.
 */
#include <stdio.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <pkru.h>
#include "shared_malloc.h"

// Size of each memory pool.
#define POOL_SIZE 256*1048576 // 256 MBs

// Memory pools used in shared malloc.
static struct sh_memory_pool* pools[DOMAINS] = {0};

int fprintf(FILE* restrict stream, const char* restrict fmt, ...) {
    int pkru = __rdpkru();
    if (pkru) {
        __wrpkru(0x0);
    }

    va_list ap;
    va_start(ap, fmt);
    int ret = vfprintf(stream, fmt, ap);
    va_end(ap);

    if (pkru) {
        __wrpkrumem(pkru);
    }

    return ret;
}

int printf(const char* restrict fmt, ...) {
    int pkru = __rdpkru();
    if (pkru) {
        __wrpkru(0x0);
    }

    va_list ap;
    va_start(ap, fmt);
    int ret = vprintf(fmt, ap);
    va_end(ap);

    if (pkru) {
        __wrpkrumem(pkru);
    }

    return ret;
}

void * (*original_malloc)(size_t) = 0;
void* malloc(size_t a0) {
    int pkru = __rdpkru();
    int domain = PKRU_TO_DOMAIN(pkru);
    if (pkru) {

        __wrpkru(0x0);

        if (!pools[domain]) {
            void* pool = mmap(0, POOL_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
            if (pool == MAP_FAILED) {
                return NULL;
            }
            if (pkey_mprotect(pool, POOL_SIZE, PROT_READ|PROT_WRITE, domain)) {
                return NULL;
            }
            pools[domain] = init_sh_mempool(pool, POOL_SIZE);
            if (pools[domain] == NULL) {
                return NULL;
            }
        }

        void* ret = sh_malloc(a0, pools[domain]);

        __wrpkrumem(pkru);

        return ret;

    } else {
        if (original_malloc == 0) {
            original_malloc = dlsym(RTLD_NEXT, "malloc");
        }

        return (*original_malloc)(a0);
    }
}

void (*original_free)(void *) = 0;
void free(void * a0) {
    int pkru = __rdpkru();
    int domain = PKRU_TO_DOMAIN(pkru);
    if (pkru) {
        __wrpkru(0x0);
        sh_free(a0, pools[domain]);
        __wrpkrumem(pkru);
    } else {
         if (original_free == 0) {
            original_free = dlsym(RTLD_NEXT, "free");
        }
        (*original_free)(a0);
    }
}