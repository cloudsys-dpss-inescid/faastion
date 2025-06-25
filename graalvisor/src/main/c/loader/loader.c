#define _GNU_SOURCE

#include "pkru.h"

#include <time.h>
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
    printf("getenv: %s\n", name);
    char *val = original_getenv(name);
    if (val)
        retval = strdup(val);
    printf("getenv result: %p\n", (void *)val);
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
    printf("secure_getenv\n");
    char *val = original_secure_getenv(name);
    if (val)
        retval = strdup(val);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

struct tm *(*original_localtime)(const time_t *) = NULL;

struct tm *localtime(const time_t *__timer) {
    struct tm *retval = NULL;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;

    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554; 

    if (original_localtime == NULL) {
        original_localtime = dlsym(RTLD_NEXT, "localtime");
    }

    __wrpkrumem(privileged_domain);
    struct tm *time = original_localtime(__timer);
    if (time) {
        retval = malloc(sizeof(struct tm));
        memcpy(retval, time, sizeof(struct tm));
    }
    __wrpkrumem(unprivileged_domain);

    return retval;
}

int (*original_pthread_create)(pthread_t *, const pthread_attr_t *, void *(*)(void *), void *) = NULL;

int
pthread_create(
    pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
    int retval;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;

    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554; 

    if (original_pthread_create == NULL) {
        original_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
    }

    __wrpkrumem(privileged_domain);
    retval = original_pthread_create(thread, attr, start_routine, arg);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

void DLL_init() {
    printf("loader stderr value: %p\n", (void *)(FILE *)stderr);

    if (original__tls_get_addr == NULL) {
        original__tls_get_addr = dlsym(RTLD_NEXT, "__tls_get_addr");
    }

    if (original_getenv == NULL) {
        original_getenv = dlsym(RTLD_NEXT, "getenv");
    }

    if (original_secure_getenv == NULL) {
        original_secure_getenv = dlsym(RTLD_NEXT, "secure_getenv");
    }

    if (original_localtime == NULL) {
        original_localtime = dlsym(RTLD_NEXT, "localtime");
    }

    if (original_pthread_create == NULL) {
        original_pthread_create = dlsym(RTLD_NEXT, "pthread_create");
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
