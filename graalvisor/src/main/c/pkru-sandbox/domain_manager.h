#ifndef DOMAIN_MANAGER_H
#define DOMAIN_MANAGER_H

#include <stdatomic.h>
#include "memory_map.h"
#include "pkru.h"

/**
 * @brief Structure representing a memory protection domain
 */ 
typedef struct Domain {
    void*               arena;              // Pointer to the memory arena
    atomic_intptr_t     function;           // Function being executed inside this domain
    IsolateFunction*    prev_function;      // Last function executed inside this domain
    pthread_mutex_t     prev_function_lock;
    IsolateFunction*    primary_function;   // overcome the hard limit of 16 namespaces in dlmopen
    // TODO - define worker_t here
} Domain;


/**
 * @brief Get the memory arena for a specific domain.
 * 
 * @param pkey The protection key of the domain from which to retrieve the arena.
 * @return void* Pointer to the memory arena of the specified domain.
 */
void *get_domain_arena(int pkey);

/**
 * @brief Initialize a domain with a specific protection key (pkey).
 * 
 * @param pkey The protection key associated with the domain.
 * @return int 0 on success, -1 on error.
 */
int initialize_domain(int pkey);

/**
 * @brief Initialize all domains.
 * 
 * @return int 0 on success, -1 on error.
 */
int initialize_all_domains();

void set_primary_pkru_sandbox(int domain, IsolateFunction *function);

IsolateFunction *get_primary_pkru_sandbox(int domain);

IsolateFunction *get_pkru_sandbox(int domain);

void cancel_domain_booking(IsolateFunction *function);

int swap_sandbox_domain(int domain, IsolateFunction *expected, IsolateFunction *function);

int get_domain_usage();

/**
 * @brief Find and book an available domain for a thread.
 * 
 * @return int The index of the booked domain, or 0 if no available domain is found.
 */
int book_available_domain(IsolateFunction *function);

/**
 * @brief Cleanup and destroy all domains and their resources.
 * 
 * This function destroys all mutexes, unmaps memory arenas, and frees allocated resources
 * for all domains. It should be called when the domains are no longer needed.
 */
void cleanup_domains();

extern Domain *domains[DOMAINS];

#endif // DOMAIN_MANAGER_H
