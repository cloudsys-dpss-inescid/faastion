#include "memory_map.h"
#include "domain_manager.h"

#include <stdio.h>

static atomic_int active_waiting_count;
static atomic_int in_queue;

// FIXME: what happens if clone operation is not successful?
void increment_sandbox_threads(IsolateFunction *function) {
    if (function == NULL)
        return;
    pthread_mutex_lock(&function->mutex);
    function->jni_threads += 1;
    pthread_mutex_unlock(&function->mutex);
}

// FIXME: what happens if thread exits via signal
void decrement_sandbox_threads(IsolateFunction *function) {
    if (function == NULL)
        return;
    leave_sandbox_domain(function);
}

void leave_sandbox_domain(IsolateFunction *function) {
    pthread_mutex_lock(&function->mutex);
    int current = function->current_domain;
    function->jni_threads -= 1;
    if (function->jni_threads == 0) {
        swap_sandbox_domain(function->current_domain, function, NULL);
        function->current_domain = 0;
    }
    pthread_mutex_unlock(&function->mutex);
}

void start_active_waiting_count() {
    atomic_init(&active_waiting_count, 0);
    atomic_init(&in_queue, 0);
}

int reset_active_waiting_count(int threshold) {
    int prev = atomic_fetch_add(&in_queue, 1);
    int place_in_queue = prev++;
    
    int t = atomic_load(&active_waiting_count); // make sure 1st in queue reads the updated value
    if (place_in_queue == 1 && t > threshold) {
        atomic_store(&active_waiting_count, 0);
        atomic_store(&in_queue, 0);
        return 1; // tell PKUSandboxProvider to use process isolation
    }
    
    return 0;
}

int get_active_waiting_count() {
    return (int)atomic_load(&active_waiting_count);
}

int enter_sandbox_domain(IsolateFunction *function) {
    int domain;
    pthread_mutex_lock(&function->mutex);
    domain = function->current_domain;
    while (domain == 0 && (domain = book_available_domain(function)) == 0) {
        atomic_fetch_add(&active_waiting_count, 1);
        usleep(100);
    }
    // printf("Chosen domain: %d\n", domain);
    function->current_domain = domain;
    function->prev_domain = domain;
    function->jni_threads += 1;
    pthread_mutex_unlock(&function->mutex);
    return domain;
}