#ifndef MEMORY_REGION_LIST_H
#define MEMORY_REGION_LIST_H

#include <stddef.h>

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

typedef struct Bucket {
    char *appName;
    MemoryRegionNode* regions; 
    struct Bucket *next;
} Bucket;

typedef struct {
    int size;
    Bucket **buckets;
} HashTable;

void init_hash_table(int size);

MemoryRegionNode *get_app_regions(char *appName);

void protect_app_regions(char *appName, int pkey);

void insert_app_region(char *appName, void* address, size_t size, int prot);

void remove_app_region(char *appName, void *address, size_t size);

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
