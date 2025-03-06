#define _GNU_SOURCE

#include "pkru.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>

static void *(*original__tls_get_addr)(void *) = NULL;

void *__tls_get_addr(void /* tls_index */ *ti) {
    void *retval;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;
    
    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554; 

    if (original__tls_get_addr == NULL) {
        original__tls_get_addr = dlsym(RTLD_NEXT, "__tls_get_addr");
    }
    
    __wrpkrumem(privileged_domain);
    retval = original__tls_get_addr(ti);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

void DLL_init() {
    if (original__tls_get_addr == NULL) {
        original__tls_get_addr = dlsym(RTLD_NEXT, "__tls_get_addr");
    }
}

void *DLL_open(const char *lib_name) {
    void *lib = dlopen(lib_name, RTLD_LAZY);
	if (!lib) {
        printf("Error loading %s: %s\n", lib_name, dlerror());
        return NULL;
    }
	return lib;
}

void *DLL_sym(void *lib, const char *name) {
	void *sym = (void *)dlsym(lib, name);
    if (!sym) {
        printf("Error resolving symbol %s: %s\n", name, dlerror());
        return NULL;
    }
	return sym;
}
