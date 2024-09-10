#ifndef DOMAIN_MANAGER_H
#define DOMAIN_MANAGER_H

#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>


/**
 * @brief Structure representing a memory protection domain
 */ 
typedef struct Domain {
    pthread_mutex_t mutex;      // Mutex to protect the domain's resources
    void*           arena;      // Pointer to the memory arena
    atomic_int      children;   // Atomic counter for tracking the number of child threads
    // TODO - define worker_t here
} Domain;


/**
 * @brief Increment the children counter atomically.
 * 
 * @param pkey the protection key of the domain whose children counter needs to be incremented.
 */
void increment_children(int pkey);

/**
 * @brief Decrement the children counter atomically.
 * 
 * @param pkey The protection key of the domain whose children counter needs to be decremented.
 */
void decrement_children(int pkey);

/**
 * @brief Get the current value of the children counter atomically.
 * 
 * @param pkey The protection key of the domain whose children counter is to be read.
 * @return int The current number of children.
 */
int get_children_count(int pkey);

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

/**
 * @brief Get the domain associated with a specific thread.
 * 
 * @param tid The thread ID to search for within the domains.
 * @return int The index of the domain associated with the thread, or -1 if the thread is not found in any domain.
 */
int get_thread_domain(pid_t tid);

/**
 * @brief Assign a thread to a specific domain.
 * 
 * @param tid The thread ID to assign to the domain.
 * @param domain The index of the domain to which the thread should be assigned.
 */
void assign_thread_to_domain(pid_t tid, int domain);

/**
 * @brief Remove a thread from a specific domain.
 * 
 * @param tid The thread ID to remove from the domain.
 * @param domain The index of the domain from which the thread should be removed.
 */
void remove_thread_from_domain(pid_t tid, int domain);

/**
 * @brief Find and book an available domain for a thread.
 * 
 * @return int The index of the booked domain, or 0 if no available domain is found.
 */
int book_available_domain();

/**
 * @brief Cleanup and destroy all domains and their resources.
 * 
 * This function destroys all mutexes, unmaps memory arenas, and frees allocated resources
 * for all domains. It should be called when the domains are no longer needed.
 */
void cleanup_domains();

#endif // DOMAIN_MANAGER_H
