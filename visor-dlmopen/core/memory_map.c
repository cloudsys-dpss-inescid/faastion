#define _GNU_SOURCE

#include "memory_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


// Head of the linked list
static MemoryRegionNode* head = NULL;

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

void append_memory_region_node(void* address, size_t size, int prot) {
    MemoryRegionNode* newNode = create_memory_region_node(address, size, prot);
    
    if (head == NULL) {
        head = newNode;
        return;
    }

    MemoryRegionNode* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;
}

void delete_memory_region_node(void *address, size_t size) {
    if (head == NULL) {
        fprintf(stderr, "error: could not delete memory region node\n");
        exit(EXIT_FAILURE);
    }

    MemoryRegionNode **nodePtr = &head;
    MemoryRegionNode *current = head;
    while (current->next != NULL) {
        if (current->region.address == address) {
            if (current->region.size != size)
                fprintf(stdout, "delete_memory_region_node: warning: sizes dont match\n"); // FIXME what should we do in this case?
            *nodePtr = current->next;
            free(current);
            return;
        }
        nodePtr = &current->next;
        current = current->next;
    }

    // is this possible?
    fprintf(stdout, "delete_memory_region_node: warning: node not found\n");
}

void print_memory_regions() {
    MemoryRegionNode* current = head;
    while (current != NULL) {
        printf("Address: %p, Size: %zu, Prot: %d\n", current->region.address, current->region.size, current->region.prot);
        current = current->next;
    }
}

void protect_memory_regions(int pkey) {
    MemoryRegionNode* current = head;
    while (current != NULL) {
        fprintf(stdout, "Protecting region: address %p, size %zu, prot %d with pkey %d\n", 
                current->region.address, current->region.size, current->region.prot, pkey);

        if (pkey_mprotect(current->region.address, current->region.size, current->region.prot, pkey) != 0) {
            perror("pkey_mprotect\n");
            fprintf(stderr, "error: failed to protect memory region with pkey %d\n", pkey);
            exit(EXIT_FAILURE);
        }
        current = current->next;
    }
}

void free_memory_region_list() {
    MemoryRegionNode* current = head;
    MemoryRegionNode* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }

    head = NULL;
}
