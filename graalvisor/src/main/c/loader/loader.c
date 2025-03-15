#define _GNU_SOURCE

#include "pkru.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

static char *(*original_getenv)(const char *) = NULL;

char *getenv(const char *name) {
    char *retval = NULL;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;

    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554; 

    if (original_getenv == NULL) {
        original_getenv = dlsym(RTLD_NEXT, "getenv");
    }

    __wrpkrumem(privileged_domain);
    char *val = original_getenv(name);
    if (val)
        retval = strdup(val);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

static char *(*original_secure_getenv)(const char *) = NULL;

char *secure_getenv(const char *name) {
    char *retval = NULL;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;

    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554;
    
    if (original_secure_getenv == NULL) {
        original_secure_getenv = dlsym(RTLD_NEXT, "secure_getenv");
    }

    __wrpkrumem(privileged_domain);
    char *val = original_secure_getenv(name);
    if (val)
        retval = strdup(val);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

void DLL_init() {
    if (original__tls_get_addr == NULL) {
        original__tls_get_addr = dlsym(RTLD_NEXT, "__tls_get_addr");
    }

    if (original_getenv == NULL) {
        original_getenv = dlsym(RTLD_NEXT, "getenv");
    }

    if (original_secure_getenv == NULL) {
        original_secure_getenv = dlsym(RTLD_NEXT, "secure_getenv");
    }
}

void *DLL_open(const char *lib_name) {
    char *error;
    dlerror();
    void *lib = dlopen(lib_name, RTLD_LAZY);
	if ((error = dlerror()) != NULL) {
        printf("Error loading %s: %s\n", lib_name, error);
        return NULL;
    }
	return lib;
}

void *DLL_sym(void *lib, const char *name) {
    char *error;
    dlerror();
	void *sym = (void *)dlsym(lib, name);
    if ((error = dlerror()) != NULL) {
        printf("Error resolving symbol %s: %s\n", name, error);
        return NULL;
    }
	return sym;
}
