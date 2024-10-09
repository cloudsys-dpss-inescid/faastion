#define _GNU_SOURCE

#include "domain_manager.h"
#include "pkru_sandbox.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <assert.h>


// return first non zero
#define COALESCE(a, b, c) ({\
int res;\
res = (res = (a)) ? res : (res = (b)) ? res : (c);\
})

// Global array of domains
Domain *domains[DOMAINS];
static int PAGE_SIZE;


void set_primary_domain_function(int domain, IsolateFunction *function) {
    domains[domain]->primary_function = function;
}

IsolateFunction *get_primary_domain_function(int domain) {
    return (IsolateFunction *)domains[domain]->primary_function;
}

void* get_domain_arena(int pkey)
{
    return domains[pkey]->arena;
}

IsolateFunction *get_domain_function(int domain) {
    return (IsolateFunction *)domains[domain]->function;
}

void cancel_domain_booking(IsolateFunction *function) {
    int domain = function->prev_domain;
    pthread_mutex_lock(&domains[domain]->prev_function_lock);
    if (domains[domain]->prev_function == function) {
        protect_app_regions(function, 0);
        domains[domain]->prev_function = NULL;
    }
    pthread_mutex_unlock(&domains[domain]->prev_function_lock);
}

int swap_domain_function(int domain, IsolateFunction *expected, IsolateFunction *function) {
    if (domain == 0)
        return 0;

    return atomic_compare_exchange_strong(
            &domains[domain]->function,
            &expected,
            (atomic_intptr_t)function);
}

int book_any_domain(IsolateFunction *function) {
    IsolateFunction *prev_function;
    for (int i = 2; i < DOMAINS; i++) {
        if (swap_domain_function(i, NULL, function)) {
            pthread_mutex_lock(&domains[i]->prev_function_lock);
            prev_function = domains[i]->prev_function;
            if (prev_function) {
                protect_app_regions(prev_function, 0);
            }
            domains[i]->prev_function = function;
            pthread_mutex_unlock(&domains[i]->prev_function_lock);
            protect_app_regions(function, i);
            return i;
        }
    }
    return 0;
}

int book_unused_domain(IsolateFunction *function) {
    for (int i = 2; i < DOMAINS; i++) {
        if (domains[i]->prev_function == NULL && swap_domain_function(i, NULL, function)) {
            pthread_mutex_lock(&domains[i]->prev_function_lock);
            domains[i]->prev_function = function;
            pthread_mutex_unlock(&domains[i]->prev_function_lock);
            protect_app_regions(function, i);
            return i;
        }
    }
    return 0;
}

int book_previous_domain(IsolateFunction *function) {
    int domain = function->prev_domain;
    if (domain == 0)
        return 0;

    if (domains[domain]->prev_function == function &&
        swap_domain_function(domain, NULL, function))
    {
        pthread_mutex_lock(&domains[domain]->prev_function_lock);
        domains[domain]->prev_function = function;
        pthread_mutex_unlock(&domains[domain]->prev_function_lock);
        return domain;
    }
    return 0;
}

int book_available_domain(IsolateFunction *function)
{
    return COALESCE(book_previous_domain(function),
            book_unused_domain(function),
            book_any_domain(function));
}

int initialize_domain(int pkey)
{
    Domain *domain = (Domain *)malloc(sizeof(Domain));
    if (!domain) {
        fprintf(stderr, "error: failed to allocate memory for domain %d\n", pkey);
        return -1;
    }

    // Allocate memory for the arena
    void *arena = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (arena == MAP_FAILED) {
        fprintf(stderr, "error: failed to mmap arena for domain %d\n", pkey);
        free(domain);
        return -1;
    }
    memset(arena, 0, PAGE_SIZE);
    
    // Populate domain
    domain->arena = arena;
    atomic_init(&domain->function, (atomic_intptr_t) NULL);
    pthread_mutex_init(&domain->prev_function_lock, NULL);
    domain->primary_function = NULL;
    domain->prev_function = NULL;
    domains[pkey] = domain;

    // Allocate protection key
    int allocated_pkey = syscall(SYS_pkey_alloc, 0, 0);
    if (allocated_pkey < 0) {
        fprintf(stderr, "error: failed to allocate pkey %d\n", pkey);
        return -1;
    }
    assert(allocated_pkey == pkey && "Protection key mismatch");

    // Protect the arena with the allocated pkey
    if (pkey_mprotect(arena, PAGE_SIZE, PROT_READ | PROT_WRITE, pkey)) {
        fprintf(stderr, "error: failed to protect arena at %p with pkey %d\n", arena, pkey);
        return -1;
    }
    // fprintf(stdout, "Protected arena at %p - %p with pkey %d\n", arena, (char*)arena + PAGE_SIZE, pkey);

    return 0;
}

int initialize_all_domains()
{
    PAGE_SIZE = getpagesize();
    for (int i = LOADER_DOMAIN; i < DOMAINS; i++) {
        if (initialize_domain(i) == -1) {
            fprintf(stderr, "error: failed to initialize domain %d\n", i);
            return -1;
        }
    }
    return 0;
}

void cleanup_domains()
{
    for (int i = 0; i < DOMAINS; i++) {
        if (domains[i]) {
            if (munmap(domains[i]->arena, PAGE_SIZE)) {
                perror("munmap");
            }

            // Free the domain structure
            free(domains[i]);

            domains[i] = NULL;
        }
    }
}
