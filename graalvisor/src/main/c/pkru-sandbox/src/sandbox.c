#include "memory_map.h"
#include "domain_manager.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static __thread IsolateFunction *isolate_function = NULL;

IsolateFunction *create_isolate_function() {
    IsolateFunction *function = (IsolateFunction *)malloc(sizeof(IsolateFunction));
    if (!function) {
        fprintf(stderr, "Could not allocate isolate function\n");
        exit(1);
    }
    pthread_mutex_init(&function->mutex, NULL);
    function->dl_handle = NULL;
    function->regions = NULL;
    function->jni_threads = 0;
    function->current_domain = 0;
    function->prev_domain = 0;
    function->notif_fd = 0;
    return function;
}

void set_isolate_function(IsolateFunction *function) {
    isolate_function = function;
}

IsolateFunction *get_isolate_function() {
    return isolate_function;
}

void destroy_isolate_function(IsolateFunction *function) {
    cancel_domain_booking(function);
    if (dlclose(function->dl_handle)) {
        fprintf(stderr, "dlclose error\n");
    }
    free_memory_region_list(function->regions);
    pthread_mutex_destroy(&function->mutex);
    free(function);
}