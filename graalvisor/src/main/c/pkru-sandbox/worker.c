#define _GNU_SOURCE

#include "pkru_sandbox.h"
#include "cr_malloc.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <asm/prctl.h>

#include <sys/mman.h>
#include <sys/syscall.h>

static __thread int _worker_domain = 0;
static __thread int _running_untrusted = 0;

void notify_worker(int domain) {
    print_systime();
    sem_post(&(worker_threads[domain].request));
}

void wait_worker(int domain) {
    sem_wait(&(worker_threads[domain].response));
    print_systime();
}

int worker_domain() {
    return _worker_domain;
}

void set_running_untrusted(int val) {
    _running_untrusted = val;
}

int is_running_untrusted() {
    return _running_untrusted;
}

int pkru_sandbox_call(int domain, void** ret, size_t* ret_size, void (*fun)(int), void *argv[], int argc)
{
    // The domain arena is setup in the following way:
    // |--- request (sizeof(request_t bytes) long) ---|--- arg (arg_size bytes long) ---|--- ret (ret_size bytes long) ---|
    request_t* request = (request_t*) get_domain_arena(domain);
    void **args = (void **) ((char*)request + sizeof(request_t));

    pthread_mutex_t *request_lock = &(worker_threads[domain].request_lock);
    pthread_mutex_lock(request_lock);

    // request->arg = (void*) ((char*)request + sizeof(request_t));
    request->num_args = argc;
    request->fun = fun;

    // Copying the function call argument to arena.
    // memcpy(request->arg, arg, arg_size);
    
    notify_worker(domain);
    wait_worker(domain);
    fprintf(stderr, "User thread notify\n");
   
    // Copying return value to arena and setting ret and ret_size pointers.
    *ret = (void*) ((char*)request + sizeof(request_t));
    *ret_size = request->ret_size;
    memcpy(*ret, request->ret, request->ret_size);

    pthread_mutex_unlock(request_lock);
	return 0;
}

void* worker(void* arg)
{
    int pkey = (int) ((long) arg);
    register_worker_thread(pkey, syscall(__NR_gettid));
    _worker_domain = pkey;

    fprintf(stderr, "Worker for domain %d is running...\n", pkey);

    sem_t *request = &(worker_threads[pkey].request);
    sem_t *response = &(worker_threads[pkey].response);
    pthread_mutex_t *request_lock = &(worker_threads[pkey].request_lock);

    sem_init(request, 0, 0);
    sem_init(response, 0, 0);
    pthread_mutex_init(request_lock, NULL);

    request_t* arena_request = (request_t*) get_domain_arena(pkey);
    for (;;) {
        sem_wait(request);
        print_systime();
        // fprintf(stderr, "Worker thread for domain %d notify\n", pkey);

        // calling native function generated in javassist 
        arena_request->fun(pkey);

        print_systime();
        sem_post(response);
    }
    return NULL;
}

// This function installs a seccomp filter in the wrapper thread to protect
// the memory regions allocated in `pthread_create`.
// This method isolates the stack, TLS, and DTV of the child thread (worker)
void* worker_wrapper(void* arg)
{
    int pkey = (int) ((long) arg);
    register_worker_thread(pkey, syscall(__NR_gettid));
    monitor_threads[pkey].seccomp_fd = install_jni_filter();
    if (pthread_create(&(worker_threads[pkey].thread), NULL, worker, (void*)(intptr_t)pkey)) {
        fprintf(stderr, "Error creating worker thread for domain %d\n", pkey);
        cleanup_and_exit();
    }
    pthread_join(worker_threads[pkey].thread, NULL);
    return NULL;
}