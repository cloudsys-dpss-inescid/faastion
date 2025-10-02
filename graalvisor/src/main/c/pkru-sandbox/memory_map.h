#ifndef MEMORY_REGION_LIST_H
#define MEMORY_REGION_LIST_H

#include <unistd.h>
#include <stddef.h>
#include <pthread.h>

/**
 * @brief Structure representing a memory region.
 */
typedef struct {
    void*  address; /** Start address of the memory region */
    size_t size;    /** Size of the memory region */
    int    prot;    /** Protection flags of the memory region */
} MemoryRegion;

/**
 * @brief Node in a linked list of memory regions.
 */
typedef struct MemoryRegionNode {
    MemoryRegion             region; /** Memory region data */
    struct MemoryRegionNode* next;   /** Pointer to the next node in the list */
} MemoryRegionNode;

typedef struct {
    pthread_mutex_t mutex;
    void *dl_handle;
    void *dl_sym;
    void *native_method;
    int current_domain;             
    int prev_domain;
    int jni_threads;
    volatile int notif_fd;
    MemoryRegionNode *regions;
} IsolateFunction;

typedef struct Bucket {
    pid_t tid;
    IsolateFunction *function;
    struct Bucket *next;
} Bucket;

typedef struct {
    int size;
    Bucket **buckets;
} HashTable;

void protect_app_regions(IsolateFunction *function, int pkey);

void insert_app_region(IsolateFunction *function, void* address, size_t size, int prot);

void protect_app_region(IsolateFunction *function, void *address, size_t size, int prot);

void remove_app_region(IsolateFunction *function, void *address, size_t size);

void leave_sandbox_domain(IsolateFunction *function);

int enter_sandbox_domain(IsolateFunction *function);

void increment_sandbox_threads(IsolateFunction *function);

void decrement_sandbox_threads(IsolateFunction *function);

IsolateFunction *create_pkru_sandbox();

void set_cached_pkru_sandbox(IsolateFunction *function);

IsolateFunction *get_cached_pkru_sandbox();

void destroy_pkru_sandbox(IsolateFunction *function);


void start_active_waiting_count();

int reset_active_waiting_count(int threshold);

int get_active_waiting_count();

/**
 * @brief Create a new MemoryRegionNode.
 * 
 * @param address The start address of the memory region.
 * @param size The size of the memory region.
 * @param prot The protection flags for the memory region.
 * @return MemoryRegionNode* Pointer to the newly created node.
 */
MemoryRegionNode* create_memory_region_node(void* address, size_t size, int prot);

/**
 * @brief Protect all memory regions in the linked list with the specified pkey.
 * 
 * @param pkey The protection key to use for memory protection.
 */
void protect_memory_regions(MemoryRegionNode *head, int pkey);

/**
 * @brief Print all memory regions in the linked list.
 */
void print_memory_regions();

/**
 * @brief Free all memory regions in the linked list and reset the list.
 */
void free_memory_region_list(MemoryRegionNode *head);

#endif // MEMORY_REGION_LIST_H
