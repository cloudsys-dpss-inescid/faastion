#define _GNU_SOURCE

#include "jni_handle.h"
#include "cr_malloc.h"
#include "pkru_sandbox.h"
#include "domain_manager.h"

#include <stdio.h>
#include <dlfcn.h>

#ifndef LOADER_LIB
#error "LOADER_LIB is not defined. Export LOADER_LIB first: it should point to core/build/libs"
#endif

static __thread void *(*DLL_open)(const char *) = NULL;
static __thread void *(*DLL_sym)(void *, const char *, void *) = NULL;

// These symbols are resolved at runtime via LD_PRELOAD
extern __attribute__((weak)) pid_t (*get_cached_tid)(void);
extern __attribute__((weak)) void (*set_cached_tid)(pid_t);
__attribute__((weak)) void *get_mstate();
__attribute__((weak)) mspace get_mspace(unsigned int pkey);
__attribute__((weak)) void *get_mspace_lock(unsigned int pkey);

extern char msids[0x400001];

static int open_loader(unsigned int domain) {
    char *error;

    dlerror();
#ifdef NO_ISOLATION
    void *dl_handle = dlopen(LOADER_LIB, RTLD_LAZY);
#else
    void *dl_handle = dlmopen(LM_ID_NEWLM, LOADER_LIB, RTLD_LAZY);
#endif
    if ((error = dlerror()) != NULL) {
        fprintf(stdout, "Could not load library: %s: %s\n", LOADER_LIB, error);
        return -1;   
    }

    dlerror();
    DLL_open = dlsym(dl_handle, "DLL_open");    
    if ((error = dlerror()) != NULL) {
        fprintf(stdout, "Failed to find the symbol: DLL_open: %s\n", error);
        return -1;
    }

    dlerror();
    DLL_sym = dlsym(dl_handle, "DLL_sym");    
    if ((error = dlerror()) != NULL) {
        fprintf(stdout, "Failed to find the symbol: DLL_sym: %s\n", error);
        return -1;
    }

    void (*DLL_worker_mspace_init)(unsigned int, void *, void *, void *, char *, pid_t (*)(void), void (*)(pid_t)) = DLL_sym(dl_handle, "worker_mspace_init", dlsym);
    if (!DLL_worker_mspace_init)
        return -1;

    DLL_worker_mspace_init(domain, get_mspace(domain), get_mspace_lock(domain), get_mstate(), msids, get_cached_tid, set_cached_tid);

    return 0;
}

// TODO: return list of handles
static void *get_handle(IsolateFunction *function) {
#ifdef REMOVE_NNS_LIMIT
    int domain = function->current_domain;
    IsolateFunction *primary_function = get_primary_pkru_sandbox(domain);
    if (primary_function)
        return primary_function->dl_handle;
#endif
    return function->dl_handle;
}

// TODO: iterate over list of handles to find the symbol
int load_native_method(IsolateFunction *function, const char *symbol) {
    void *dl_handle = NULL;
    
    if ((dl_handle = get_handle(function)) == NULL)
        return -1;

    void *native_method = (void *)DLL_sym(dl_handle, symbol, dlsym);
    if (!native_method)
        return -1;

    function->native_method = native_method;

    return 0;
}

void load_native_library(IsolateFunction *function, const char *filename) {
    int domain;
    
    domain = function->current_domain;
#ifdef REMOVE_NNS_LIMIT
    IsolateFunction *primary_function = get_primary_pkru_sandbox(domain);
    if (primary_function) {
        return;
    }
#endif

    if (DLL_open == NULL) {
        open_loader(domain);
    }

    set_running_untrusted(1);
    if ((function->dl_handle = DLL_open(filename)) == NULL) {
        fprintf(stderr, "Error loading native library: %s\n", filename);
        exit(1);
    }

#ifdef REMOVE_NNS_LIMIT
    if (function->dl_handle)
        set_primary_pkru_sandbox(domain, function);
#endif

    return;
}