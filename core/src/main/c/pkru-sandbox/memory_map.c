#define _GNU_SOURCE

#include "memory_map.h"
#include "domain_manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

void delete_memory_region_node(MemoryRegionNode **nodePtr, void *address, size_t size);

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

/**
 * @brief Append a new MemoryRegionNode to the end of the linked list.
 * 
 * @param address The start address of the new memory region.
 * @param size The size of the new memory region.
 * @param prot The protection flags for the new memory region.
 */
void append_memory_region_node(MemoryRegionNode** head, void* address, size_t size, int prot) {
    MemoryRegionNode* newNode = create_memory_region_node(address, size, prot);

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

int protect_memory_region_node(MemoryRegionNode **nodePtr, void *address, size_t size, int prot) {
    MemoryRegionNode *head = *nodePtr;
    if (head == NULL)
        return size;

    MemoryRegionNode *current = head;
    int prot_flags;
    size_t bytes_left;
    unsigned long addr;
    unsigned long mem_end_addr;
    unsigned long mem_start_addr;
    unsigned long mem_split_start_addr;
    bytes_left = size;
    addr = (unsigned long)address;
    mem_split_start_addr = (unsigned long)address + size;
    while (current != NULL) {
        mem_start_addr = (unsigned long)current->region.address;
        mem_end_addr = (unsigned long)mem_start_addr + current->region.size;
        if (addr == mem_start_addr) {
            if (mem_split_start_addr == mem_end_addr) {
                current->region.prot = prot;
            } else if (mem_split_start_addr < mem_end_addr) {
                prot_flags = current->region.prot;
                current->region.size = mem_split_start_addr - mem_start_addr;
                current->region.prot = prot;
                append_memory_region_node(nodePtr, (void *)mem_split_start_addr,
                    mem_end_addr - mem_split_start_addr, prot_flags);
            } else {
                delete_memory_region_node(nodePtr, (void *)mem_end_addr, 
                    mem_split_start_addr - mem_end_addr);
                current->region.size = size;
                current->region.prot = prot;
            }
            return 0;
        } else if (addr > mem_start_addr && addr < mem_end_addr) {
            if (mem_split_start_addr == mem_end_addr) {
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(nodePtr, (void *)addr,
                    mem_split_start_addr - addr, prot);
            } else if (mem_split_start_addr < mem_end_addr) {
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(nodePtr, (void *)mem_split_start_addr,
                    mem_end_addr - mem_split_start_addr, current->region.prot);
                append_memory_region_node(nodePtr, (void *)addr,
                    mem_split_start_addr - addr, prot);
            } else {
                delete_memory_region_node(nodePtr, (void *)mem_end_addr, 
                    mem_split_start_addr - mem_end_addr);
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(nodePtr, (void *)addr,
                    mem_split_start_addr - mem_start_addr, prot);
            }
            return 0;
        } else {
            current = current->next;
        }
    }

    return bytes_left;
}

void delete_memory_region_node(MemoryRegionNode **nodePtr, void *address, size_t size) {
    MemoryRegionNode *head = *nodePtr;
    if (head == NULL)
        return;

    MemoryRegionNode *current = head;
    unsigned long addr;
    unsigned long mem_end_addr;
    unsigned long mem_start_addr;
    unsigned long mem_split_start_addr;
    addr = (unsigned long)address;
    mem_split_start_addr = (unsigned long)address + size;
    while (current != NULL) {
        mem_start_addr = (unsigned long)current->region.address;
        mem_end_addr = (unsigned long)mem_start_addr + current->region.size;
        if (addr == mem_start_addr) {
            if (mem_split_start_addr == mem_end_addr) {
                *nodePtr = current->next;
                free(current);
            } else if (mem_split_start_addr < mem_end_addr) {
                current->region.address = (void *)mem_split_start_addr;
                current->region.size = mem_end_addr - mem_split_start_addr;
            } else {
                *nodePtr = current->next;
                free(current);
                delete_memory_region_node(nodePtr, (void *)mem_end_addr,
                    mem_split_start_addr - mem_end_addr);
            }
            return;
        } else if (addr > mem_start_addr && addr < mem_end_addr) {
            if (mem_split_start_addr == mem_end_addr) {
                current->region.size = addr - mem_start_addr;
            } else if (mem_split_start_addr < mem_end_addr) {
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(nodePtr, (void *)mem_split_start_addr,
                    mem_end_addr - mem_split_start_addr, current->region.prot);
            } else {
                current->region.size = addr - mem_start_addr;
                delete_memory_region_node(nodePtr, (void *)mem_end_addr,
                    mem_split_start_addr - mem_end_addr);
            }
            return;
        } else {
            nodePtr = &current->next;
            current = current->next;
        }
    }
}

// void print_memory_regions() {
//     MemoryRegionNode* current = head;
//     while (current != NULL) {
//         printf("Address: %p, Size: %zu, Prot: %d\n", current->region.address, current->region.size, current->region.prot);
//         current = current->next;
//     }
// }

void free_memory_region_list(MemoryRegionNode *head) {
    MemoryRegionNode *current = head;
    MemoryRegionNode *nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
}

void insert_app_region(IsolateFunction *function, void* address, size_t size, int prot) {
    if (function == NULL)
        return;
    size_t bytes_left = protect_memory_region_node(&function->regions, address, size, prot);
    if (bytes_left) {
        address = (void *)((char *)address + size - bytes_left);
        append_memory_region_node(&function->regions, address, bytes_left, prot);
    }
}

void remove_app_region(IsolateFunction *function, void *address, size_t size) {
    if (function == NULL)
        return;
    delete_memory_region_node(&function->regions, address, size);
}

void protect_memory_regions(MemoryRegionNode *head, int pkey) {
    MemoryRegionNode* current = head;
    while (current != NULL) {
        // fprintf(stdout, "Protecting region: address %ld-%ld, prot %d with pkey %d\n", 
        //         (unsigned long)current->region.address, 
        //         (unsigned long)current->region.address + current->region.size,
        //         current->region.prot, pkey);

        if (pkey_mprotect(current->region.address, current->region.size, current->region.prot, pkey) != 0) {
            fprintf(stderr, "error: failed to protect memory region with pkey %d\n", pkey);
            exit(EXIT_FAILURE);
        }
        current = current->next;
    }
}

void protect_app_regions(IsolateFunction *function, int pkey) {
#ifdef REMOVE_NNS_LIMIT
    int domain;

    if (pkey == 0)
        // FIXME: (temporary workaround)
        // our tests use different function names to make sure we use different domains
        // However, we do not wish to use different memory mappings
        return; 
        // domain = function->prev_domain;
    else
        domain = pkey;

    IsolateFunction *primary_function = get_primary_pkru_sandbox(domain);
    if (primary_function) {
        // FIXME: (temporary workaround)
        return;
        // protect_memory_regions(primary_function->regions, pkey);
        // return;
    }
#endif

    protect_memory_regions(function->regions, pkey);
}

void protect_app_region(IsolateFunction *function, void *address, size_t size, int prot) {
    if (function == NULL)
        return;
    protect_memory_region_node(&function->regions, address, size, prot);
}
