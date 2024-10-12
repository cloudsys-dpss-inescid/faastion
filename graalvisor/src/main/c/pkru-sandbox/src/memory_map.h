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
    int current_domain;             
    int prev_domain;
    int jni_threads;
    MemoryRegionNode *regions;
} IsolateFunction;

typedef struct Bucket {
    char *functionName;
    IsolateFunction *function;
    struct Bucket *next;
} Bucket;

typedef struct {
    int size;
    Bucket **buckets;
} HashTable;

void init_hash_table(int size);

void protect_app_regions(IsolateFunction *function, int pkey);

void insert_app_region(IsolateFunction *function, void* address, size_t size, int prot);

void protect_app_region(IsolateFunction *function, void *address, size_t size, int prot);

void remove_app_region(IsolateFunction *function, void *address, size_t size);

void leave_function_domain(IsolateFunction *function);

int enter_function_domain(IsolateFunction *function);

void clone_function_thread(IsolateFunction *function);

void join_function_thread(IsolateFunction *function);

IsolateFunction *create_isolate_function();

void destroy_isolate_function(IsolateFunction *function);

void set_isolate_function(IsolateFunction *function);

IsolateFunction *get_isolate_function();

IsolateFunction *get_app_function(const char *functionName);

void insert_app_function(const char *functionName, IsolateFunction *function);

void remove_app_function(const char *functionName);

void free_hash_table();

/**
 * @brief Create a new MemoryRegionNode.
 * 
 * @param address The start address of the memory region.
 * @param size The size of the memory region.
 * @param prot The protection flags for the memory region.
 * @return MemoryRegionNode* Pointer to the newly created node.
 */
MemoryRegionNode* create_memory_region_node(void* address, size_t size, int prot);

void delete_memory_region_node(MemoryRegionNode *head, void *address, size_t size);

/**
 * @brief Append a new MemoryRegionNode to the end of the linked list.
 * 
 * @param address The start address of the new memory region.
 * @param size The size of the new memory region.
 * @param prot The protection flags for the new memory region.
 */
void append_memory_region_node(MemoryRegionNode **head, void* address, size_t size, int prot);

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
