#ifndef DOMAIN_MANAGER_H
#define DOMAIN_MANAGER_H

#include <pthread.h>
#include <unistd.h>

/**
 * @brief Structure representing a memory protection domain
 */ 
typedef struct Domain {
    pthread_mutex_t mutex;   // Mutex to protect the domain's resources
    void*           arena;   // Pointer to the memory arena
    // TODO - keep track of number of threads
} Domain;

/**
 * @brief Get the memory arena for a specific domain.
 * 
 * @param domain The index of the domain from which to retrieve the arena.
 * @return void* Pointer to the memory arena of the specified domain.
 */
void *get_domain_arena(int domain);

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
 * @param tid The thread ID for which to book a domain.
 * @return int The index of the booked domain, or -1 if no available domain is found.
 */
int book_available_domain_for_thread(pid_t tid);

/**
 * @brief Cleanup and destroy all domains and their resources.
 * 
 * This function destroys all mutexes, unmaps memory arenas, and frees allocated resources
 * for all domains. It should be called when the domains are no longer needed.
 */
void cleanup_domains();

#endif // DOMAIN_MANAGER_H
