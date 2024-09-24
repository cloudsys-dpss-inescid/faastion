#define _GNU_SOURCE

#include "domain_manager.h"
#include "pkru_sandbox.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <assert.h>


// Global array of domains
Domain *domains[DOMAINS];
static int PAGE_SIZE;


void* get_domain_arena(int pkey)
{
    return domains[pkey]->arena;
}

void increment_children(int pkey)
{
    atomic_fetch_add(&(domains[pkey]->children), 1);
}

void decrement_children(int pkey)
{
    atomic_fetch_sub(&(domains[pkey]->children), 1);
}

int book_available_domain()
{
    int expected = 0; // We expect children count to be 0

    for (int i = 2; i < DOMAINS; i++) {
        // Try to set children to 1 only if it is currently 0
        // NOTE - need to decrement after execution is finished
        if (atomic_compare_exchange_strong(&domains[i]->children, &expected, 1)) {
            return i;
        }
    }
    return 0;
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
    pthread_mutex_init(&domain->mutex, NULL);
    atomic_init(&domain->children, 0);
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
    fprintf(stdout, "Protected arena at %p - %p with pkey %d\n", arena, (char*)arena + PAGE_SIZE, pkey);

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

            // Destroy the mutex
            pthread_mutex_destroy(&domains[i]->mutex);
            
            // Free the domain structure
            free(domains[i]);

            domains[i] = NULL;
        }
    }
}
