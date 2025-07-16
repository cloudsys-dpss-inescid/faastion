#define _GNU_SOURCE

#include "pkru.h"

#include <time.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define lookup_symbol(sym) if (original_##sym == NULL) original_##sym = dlsym(RTLD_NEXT, #sym)

static void *(*original___tls_get_addr)(void *) = NULL;

void *__tls_get_addr(void /* tls_index */ *ti) {
    void *retval;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;
    
    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554; 
    
    __wrpkrumem(privileged_domain);
    lookup_symbol(__tls_get_addr);
    retval = original___tls_get_addr(ti);
    // printf("[libc] tls: %p\n", retval);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

typedef void (*dtor_func) (void *);

int (*original___cxa_thread_atexit_impl)(dtor_func, void *, void *) = NULL;

int __cxa_thread_atexit_impl (dtor_func func, void *obj, void *dso_symbol) {
    int retval;
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;
    
    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554; 
    
    __wrpkrumem(privileged_domain);
    lookup_symbol(__cxa_thread_atexit_impl);
    retval = original___cxa_thread_atexit_impl(func, obj, dso_symbol);
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

    __wrpkrumem(privileged_domain);
    lookup_symbol(getenv);
    // printf("[libc] getenv: %s\n", name);
    char *val = original_getenv(name);
    if (val)
        retval = strdup(val);
    // printf("getenv result: %p\n", (void *)val);
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

    __wrpkrumem(privileged_domain);
    lookup_symbol(secure_getenv);
    // printf("[libc] secure_getenv\n");
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

    __wrpkrumem(privileged_domain);
    lookup_symbol(localtime);
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

    __wrpkrumem(privileged_domain);
    lookup_symbol(pthread_create);
    retval = original_pthread_create(thread, attr, start_routine, arg);
    __wrpkrumem(unprivileged_domain);

    return retval;
}

void DLL_init() {}

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
