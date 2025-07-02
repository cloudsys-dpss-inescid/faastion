#define _GNU_SOURCE

#include "jni_handle.h"
#include "cr_malloc.h"
#include "pkru_sandbox.h"
#include "domain_manager.h"

#include <stdio.h>
#include <dlfcn.h>

#ifndef LOADER_LIB
#error "LOADER_LIB is not defined. Export LOADER_LIB first: it should point to graalvisor/build/libs"
#endif

void *_native_method;
static __thread void *(*DLL_open)(const char *) = NULL;
static __thread void *(*DLL_sym)(void *, const char *) = NULL;

// These symbols are resolved at runtime via LD_PRELOAD
__attribute__((weak)) mspace get_mspace(unsigned int pkey);
__attribute__((weak)) void *get_mspace_lock(unsigned int pkey);

static int open_loader(unsigned int domain) {
    char *error;

    dlerror();
    void *dl_handle = dlmopen(LM_ID_NEWLM, LOADER_LIB, RTLD_LAZY);
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

    int (*DLL_get_mspace_count)(void) = DLL_sym(dl_handle, "get_mspace_count");
    if (!DLL_get_mspace_count)
        return -1;

    int mspaces = DLL_get_mspace_count();
    printf("mspaces: %d\n", mspaces);

    void (*DLL_set_mspace_lock)(unsigned int, void *) = DLL_sym(dl_handle, "set_mspace_lock");
    if (!DLL_set_mspace_lock)
        return -1;

    void (*DLL_set_mspace)(unsigned int, void *) = DLL_sym(dl_handle, "set_mspace");
    if (!DLL_set_mspace)
        return -1;

    void (*DLL_register_worker_thread)(unsigned int, unsigned int) = DLL_sym(dl_handle, "register_worker_thread");
    if (!DLL_register_worker_thread)
        return -1;

    DLL_set_mspace(domain, get_mspace(domain));
    DLL_set_mspace_lock(domain, get_mspace_lock(domain));
    DLL_register_worker_thread(domain, gettid());

    return 0;
}

// TODO: return list of handles
static void *get_handle(IsolateFunction *function) {
    printf("enter get handle\n");
    return function->dl_handle;
}

// TODO: iterate over list of handles to find the symbol
int load_native_method(IsolateFunction *function, const char *symbol) {
    printf("enter load native method\n");
    void *dl_handle;
    
    if ((dl_handle = get_handle(function)) == NULL)
        return -1;

    printf("dl_handle: %p\n", dl_handle);

    _native_method = (void *)DLL_sym(dl_handle, symbol);    
    if (!_native_method)
        return -1;

    return 0;
}

void load_native_library(IsolateFunction *function, const char *filename) {
    printf("enter load native library\n");

    int domain;
    
    domain = function->current_domain;
#ifdef REMOVE_NNS_LIMIT
    IsolateFunction *primary_function = get_primary_pkru_sandbox(domain);
    printf("primary function: %p\n", primary_function);
    if (primary_function) {
        return;
    }
#endif

    if (DLL_open == NULL) {
        open_loader(domain);
    }

    printf("open_loader: success\n");

    set_running_untrusted(1);
    if ((function->dl_handle = DLL_open(filename)) == NULL) {
        fprintf(stderr, "Error loading native library: %s\n", filename);
        exit(1);
    }

    printf("function handle: %p\n", function->dl_handle);

#ifdef REMOVE_NNS_LIMIT
    if (function->dl_handle)
        set_primary_pkru_sandbox(domain, function);
#endif

    return;
}