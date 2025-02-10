#include "memory_map.h"
#include "domain_manager.h"

#include <stdio.h>

static atomic_int active_waiting_count;

// FIXME: what happens if clone operation is not successful?
void clone_function_thread(IsolateFunction *function) {
    if (function == NULL)
        return;
    pthread_mutex_lock(&function->mutex);
    function->jni_threads += 1;
    pthread_mutex_unlock(&function->mutex);
}

// FIXME: what happens if thread exits via signal
void join_function_thread(IsolateFunction *function) {
    if (function == NULL)
        return;
    leave_function_domain(function);
}

void leave_function_domain(IsolateFunction *function) {
    pthread_mutex_lock(&function->mutex);
    int current = function->current_domain;
    function->jni_threads -= 1;
    if (function->jni_threads == 0) {
        swap_domain_function(function->current_domain, function, NULL);
        function->current_domain = 0;
    }
    pthread_mutex_unlock(&function->mutex);
}

void start_active_waiting_count() {
    atomic_init(&active_waiting_count, 0);
}

void reset_active_waiting_count() {
    atomic_store(&active_waiting_count, 0);
}

int get_active_waiting_count() {
    return (int)atomic_load(&active_waiting_count);
}

int enter_function_domain(IsolateFunction *function) {
    int domain;
    pthread_mutex_lock(&function->mutex);
    domain = function->current_domain;
    while (domain == 0 && (domain = book_available_domain(function)) == 0) {
        atomic_fetch_add(&active_waiting_count, 1);
        usleep(100);
    }
    printf("Chosen domain: %d\n", domain);
    function->current_domain = domain;
    function->prev_domain = domain;
    function->jni_threads += 1;
    pthread_mutex_unlock(&function->mutex);
    return domain;
}