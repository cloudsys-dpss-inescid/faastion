#define _GNU_SOURCE

#include "memory_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>


// Head of the linked list
// static MemoryRegionNode* head = NULL;

static HashTable *hashTable = NULL;

MemoryRegionNode* create_memory_region_node(void* address, size_t size, int prot) {
    MemoryRegionNode* newNode = (MemoryRegionNode*)malloc(sizeof(MemoryRegionNode));
    if (!newNode) {
        perror("Failed to allocate memory for MemoryRegionNode");
        exit(EXIT_FAILURE);
    }
    newNode->region.address = address;
    newNode->region.size = size;
    newNode->region.prot = prot;
    newNode->next = NULL;
    return newNode;
}

void append_memory_region_node(MemoryRegionNode** head, void* address, size_t size, int prot) {
    MemoryRegionNode* newNode = create_memory_region_node(address, size, prot);
    if (!newNode) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    if (*head == NULL) {
        *head = newNode;
        return;
    }

    MemoryRegionNode* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

void delete_memory_region_node(MemoryRegionNode *head, void *address, size_t size) {
    if (head == NULL) {
        fprintf(stderr, "error: could not delete memory region node\n");
        exit(EXIT_FAILURE);
    }

    MemoryRegionNode **nodePtr = &head;
    MemoryRegionNode *current = head;
    while (current->next != NULL) {
        if (current->region.address == address) {
            if (current->region.size != size)
                fprintf(stdout, "delete_memory_region_node: warning: sizes dont match\n");
            *nodePtr = current->next;
            free(current);
            return;
        }
        nodePtr = &current->next;
        current = current->next;
    }

    // FIXME: what should happen
    fprintf(stdout, "delete_memory_region_node: warning: node not found\n");
}

// void print_memory_regions() {
//     MemoryRegionNode* current = head;
//     while (current != NULL) {
//         printf("Address: %p, Size: %zu, Prot: %d\n", current->region.address, current->region.size, current->region.prot);
//         current = current->next;
//     }
// }

void protect_memory_regions(MemoryRegionNode *head, int pkey) {
    MemoryRegionNode* current = head;
    while (current != NULL) {
        fprintf(stdout, "Protecting region: address %p, size %zu, prot %d with pkey %d\n", 
                current->region.address, current->region.size, current->region.prot, pkey);

        if (pkey_mprotect(current->region.address, current->region.size, current->region.prot, pkey) != 0) {
            fprintf(stderr, "error: failed to protect memory region with pkey %d\n", pkey);
            exit(EXIT_FAILURE);
        }
        current = current->next;
    }
}

void free_memory_region_list(MemoryRegionNode *head) {
    MemoryRegionNode *current = head;
    MemoryRegionNode *nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
}

unsigned int hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % hashTable->size;
}

// Function to create a hash table
void init_hash_table(int size) {
    hashTable = (HashTable *)malloc(sizeof(HashTable));
    if (!hashTable) {
        fprintf(stderr, "Could not allocate hashTable\n");
        exit(1);
    }

    hashTable->size = size;
    hashTable->buckets = (Bucket **)calloc(size, sizeof(Bucket));
    if (!hashTable->buckets) {
        free(hashTable);
        exit(1);
    }
}

Bucket *get_bucket(char *appName) {
    Bucket *currentBucket = hashTable->buckets[hash(appName)];
    while (currentBucket != NULL &&
            currentBucket->appName != NULL &&
            strcmp(currentBucket->appName, appName) != 0)
    {
        currentBucket = currentBucket->next;
    }
    return currentBucket;
}

void insert_app_region(char *appName, void* address, size_t size, int prot) {
    Bucket **head = &hashTable->buckets[hash(appName)];
    Bucket *currentBucket = get_bucket(appName);
    if (currentBucket == NULL) {
        currentBucket = (Bucket *)malloc(sizeof(Bucket));
        currentBucket->appName = strdup(appName);
        currentBucket->regions = NULL;
        currentBucket->next = *head;
        *head = currentBucket;
    }
    append_memory_region_node(&(currentBucket->regions), address, size, prot);
}


// MemoryRegionNode *get_app_regions(char *appName);

void protect_app_regions(char *appName, int pkey) {
    Bucket *currentBucket = get_bucket(appName);
    if (currentBucket == NULL) {
        fprintf(stderr, "Warning: protect_app_regions: appName does not exist\n");
        return;
    }
    protect_memory_regions(currentBucket->regions, pkey);
}

void remove_app_region(char *appName, void *address, size_t size) {
    Bucket *currentBucket = get_bucket(appName);
    if (currentBucket == NULL) {
        fprintf(stderr, "Warning: remove_app_region: appName does not exist\n");
        return;
    }
    delete_memory_region_node(currentBucket->regions, address, size);    
}

void free_hash_table() {
    for (int i = 0; i < hashTable->size; i++) {
        Bucket *currentBucket = hashTable->buckets[i];
        Bucket *nextBucket;

        while (currentBucket != NULL) {
            nextBucket = currentBucket->next;
            free_memory_region_list(currentBucket->regions);
            free(currentBucket->appName);
            free(currentBucket);
            currentBucket = nextBucket;
        }
    }

    free(hashTable->buckets);
    free(hashTable);
}
