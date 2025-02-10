#define _GNU_SOURCE

#include "pkru_sandbox.h"

#include <stdio.h>
#include <pthread.h>

#include <sys/mman.h>

void notify_worker(int domain) {
    print_systime();
    sem_post(&(worker_threads[domain].request));
}

void wait_worker(int domain) {
    sem_wait(&(worker_threads[domain].response));
    print_systime();
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
    void *stackaddr;
    size_t stacksize;
    pthread_attr_t attr;
    int pkey = (int) ((long) arg);

    monitor_threads[pkey].seccomp_fd = install_jni_filter();
    
    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, &stackaddr, &stacksize);
    pkey_mprotect(stackaddr, stacksize, PROT_READ | PROT_WRITE | PROT_EXEC, pkey);

    // FIXME: This requires a more robust solution 
    // Worker heap allocation was being added to some function memory mappings
    // Meaning that when said function moves to domain 0, this worker heap becomes unreachable
    // The malloc operation below forces the worker thread to pre allocate heap
    // It overcomes part of the problem; however, future heap allocations may still happen
    {
        void *heap;
        if ((heap = malloc(1))) {
            free(heap);
        }
    }

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