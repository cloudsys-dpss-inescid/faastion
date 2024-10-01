#define _GNU_SOURCE

#include "domain_manager.h"
#include "memory_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <dlfcn.h>


// Head of the linked list
// static MemoryRegionNode* head = NULL;

static HashTable *hashTable = NULL;

static __thread IsolateFunction *isolate_function = NULL;

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

int protect_memory_region_node(MemoryRegionNode *head, void *address, size_t size, int prot) {
    if (head == NULL)
        return size;

    MemoryRegionNode **nodePtr = &head;
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
                append_memory_region_node(&head, (void *)mem_split_start_addr,
                    mem_end_addr - mem_split_start_addr, prot_flags);
            } else {
                delete_memory_region_node(head, (void *)mem_end_addr, 
                    mem_split_start_addr - mem_end_addr);
                current->region.size = size;
                current->region.prot = prot;
            }
            return 0;
        } else if (addr > mem_start_addr && addr < mem_end_addr) {
            if (mem_split_start_addr == mem_end_addr) {
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(&head, (void *)addr,
                    mem_split_start_addr - addr, prot);
            } else if (mem_split_start_addr < mem_end_addr) {
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(&head, (void *)mem_split_start_addr,
                    mem_end_addr - mem_split_start_addr, current->region.prot);
                append_memory_region_node(&head, (void *)addr,
                    mem_split_start_addr - addr, prot);
            } else {
                delete_memory_region_node(head, (void *)mem_end_addr, 
                    mem_split_start_addr - mem_end_addr);
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(&head, (void *)addr,
                    mem_split_start_addr - mem_start_addr, prot);
            }
            return 0;
        } else {
            nodePtr = &current->next;
            current = current->next;
        }
    }

    return bytes_left;
}

void delete_memory_region_node(MemoryRegionNode *head, void *address, size_t size) {
    if (head == NULL)
        return;

    MemoryRegionNode **nodePtr = &head;
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
                delete_memory_region_node(head, (void *)mem_end_addr,
                    mem_split_start_addr - mem_end_addr);
            }
            return;
        } else if (addr > mem_start_addr && addr < mem_end_addr) {
            if (mem_split_start_addr == mem_end_addr) {
                current->region.size = addr - mem_start_addr;
            } else if (mem_split_start_addr < mem_end_addr) {
                current->region.size = addr - mem_start_addr;
                append_memory_region_node(&head, (void *)mem_split_start_addr,
                    mem_end_addr - mem_split_start_addr, current->region.prot);
            } else {
                current->region.size = addr - mem_start_addr;
                delete_memory_region_node(head, (void *)mem_end_addr,
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

void protect_memory_regions(MemoryRegionNode *head, int pkey) {
    MemoryRegionNode* current = head;
    while (current != NULL) {
        fprintf(stdout, "Protecting region: address %ld-%ld, prot %d with pkey %d\n", 
                (unsigned long)current->region.address, 
                (unsigned long)current->region.address + current->region.size,
                current->region.prot, pkey);

        if (pkey_mprotect(current->region.address, current->region.size, current->region.prot, pkey) != 0) {
            perror("pkey_mprotect");
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

IsolateFunction *create_isolate_function() {
    IsolateFunction *function = (IsolateFunction *)malloc(sizeof(IsolateFunction));
    if (!function) {
        fprintf(stderr, "Could not allocate isolate function\n");
        exit(1);
    }
    pthread_mutex_init(&function->mutex, NULL);
    function->dl_handle = NULL;
    function->regions = NULL;
    function->jni_threads = 0;
    function->current_domain = 0;
    function->prev_domain = 0;
    return function;
}

void set_isolate_function(IsolateFunction *function) {
    isolate_function = function;
}

IsolateFunction *get_isolate_function() {
    return isolate_function;
}

// FIXME: what happens if clone operation is not successful?
void clone_function_thread(IsolateFunction *function) {
    if (function == NULL)
        return;
    pthread_mutex_lock(&function->mutex);
    function->jni_threads += 1;
    pthread_mutex_unlock(&function->mutex);
}

// FIXME: what happens if thread exits via signal
void join_function_thread(IsolateFunction *function) {
    if (function == NULL)
        return;
    leave_function_domain(function);
}

static void print_file(char* filepath, char* logpath)
{
    FILE* logfile = fopen(logpath, "w");
    FILE* file = fopen(filepath, "r");
    if (!file) {
        fprintf(stderr, "Failed to open %s\n", filepath);
        exit(EXIT_FAILURE);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        fprintf(logfile, "%s", line);
    }

    fclose(logfile);
    fclose(file);
}

void destroy_isolate_function(IsolateFunction *function) {
    // print_file("/proc/self/maps", "maps_after");
    cancel_domain_booking(function);
    if (dlclose(function->dl_handle)) {
        fprintf(stderr, "dlclose error\n");
    }
    free_memory_region_list(function->regions);
    pthread_mutex_destroy(&function->mutex);
    free(function);
}

void leave_function_domain(IsolateFunction *function) {
    pthread_mutex_lock(&function->mutex);
    int current = function->current_domain;
    function->jni_threads -= 1;
    if (function->jni_threads == 0) {
        swap_domain_function(function->current_domain, function, NULL);
        function->current_domain = 0;
    }
    pthread_mutex_unlock(&function->mutex);
}

int enter_function_domain(IsolateFunction *function) {
    int domain;
    pthread_mutex_lock(&function->mutex);
    domain = function->current_domain ? function->current_domain : book_available_domain(function);
    function->current_domain = domain;
    function->prev_domain = domain;
    function->jni_threads += 1;
    pthread_mutex_unlock(&function->mutex);
    return domain;
}

unsigned int hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % hashTable->size;
}

unsigned int hash_int(int num) {
    unsigned long hash = (num * 2654435761);
    return (unsigned int)(hash & (hashTable->size - 1));
}

// Function to create a hash table
void init_hash_table(int size) {
    hashTable = (HashTable *)malloc(sizeof(HashTable));
    if (!hashTable) {
        fprintf(stderr, "Could not allocate hashTable\n");
        exit(1);
    }

    int i;
    float aux = (float)size;
    for (i = 1; aux > 2; i++) {
        aux /= 2;
    }
    size = 1 << i;

    hashTable->size = size;
    hashTable->buckets = (Bucket **)calloc(size, sizeof(Bucket));
    if (!hashTable->buckets) {
        free(hashTable);
        exit(1);
    }
}

Bucket *get_bucket(const char *functionName) {
    Bucket *currentBucket = hashTable->buckets[hash_string(functionName)];
    while (currentBucket != NULL &&
        currentBucket->functionName != NULL &&
        strcmp(currentBucket->functionName, functionName) != 0)
    {
        currentBucket = currentBucket->next;
    }
    return currentBucket;
}

void insert_app_function(const char *functionName, IsolateFunction *function) {
    Bucket **head = &hashTable->buckets[hash_string(functionName)];
    Bucket *currentBucket = get_bucket(functionName);
    if (currentBucket == NULL) {
        currentBucket = (Bucket *)malloc(sizeof(Bucket));
        currentBucket->functionName = strdup(functionName);
        currentBucket->function = function;
        currentBucket->next = *head;
        *head = currentBucket;
    }
}

void remove_app_function(const char *functionName) {
    Bucket **head = &hashTable->buckets[hash_string(functionName)];
    Bucket *currentBucket = *head;
    Bucket *previousBucket = NULL;
    while (currentBucket != NULL) {
        if (currentBucket->functionName == NULL || strcmp(currentBucket->functionName, functionName) != 0) {
            previousBucket = currentBucket;
            currentBucket = currentBucket->next;
            continue;
        } else if (currentBucket == *head) {
            *head = currentBucket->next;
        } else {
            previousBucket->next = currentBucket->next;
        }
        destroy_isolate_function(currentBucket->function);
        free(currentBucket->functionName);
        free(currentBucket);
        break;
    } 
}

IsolateFunction *get_app_function(char *functionName) {
    Bucket *currentBucket = get_bucket(functionName);
    return currentBucket ? currentBucket->function : NULL;
}

void insert_app_region(IsolateFunction *function, void* address, size_t size, int prot) {
    if (function == NULL)
        return;
    size_t bytes_left = protect_memory_region_node(function->regions, address, size, prot);
    if (bytes_left) {
        address = (void *)((char *)address + size - bytes_left);
        append_memory_region_node(&function->regions, address, bytes_left, prot);
    }
}

void protect_app_region(IsolateFunction *function, void *address, size_t size, int prot) {
    if (function == NULL)
        return;
    protect_memory_region_node(function->regions, address, size, prot);
}

void protect_app_regions(IsolateFunction *function, int pkey) {
    protect_memory_regions(function->regions, pkey);
}

void remove_app_region(IsolateFunction *function, void *address, size_t size) {
    if (function == NULL)
        return;
    delete_memory_region_node(function->regions, address, size);
}

void free_hash_table() {
    for (int i = 0; i < hashTable->size; i++) {
        Bucket *currentBucket = hashTable->buckets[i];
        Bucket *nextBucket;

        while (currentBucket != NULL) {
            nextBucket = currentBucket->next;
            destroy_isolate_function(currentBucket->function);
            free(currentBucket->functionName);
            free(currentBucket);
            currentBucket = nextBucket;
        }
    }

    free(hashTable->buckets);
    free(hashTable);
}
