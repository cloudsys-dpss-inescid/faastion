#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appmap.h"

MRNode* create_MRNode(void* address, size_t size, int flags) {
    MRNode* newNode = (MRNode*)malloc(sizeof(MRNode));
    if (!newNode) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    newNode->region.address = address;
    newNode->region.size = size;
    newNode->region.flags = flags;
    newNode->next = NULL;
    return newNode;
}

void append_MRNode(MRNode* head, void* address, size_t size, int flags) {
    MRNode* newNode = create_MRNode(address, size, flags);
    if (!head) {
        head = newNode;
    } else {
        MRNode* current = head;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void print_regions(MRNode* head) {
    MRNode* current = head;
    while (current) {
        fprintf(stderr, "Address: %p, Size: %zu, Flags: %d -> ", current->region.address, current->region.size, current->region.flags);
        current = current->next;
    }
    fprintf(stderr, "NULL\n");
}

void free_regions(MRNode* head) {
    MRNode* current = head;
    while (current) {
        MRNode* temp = current;
        current = current->next;
        free(temp);
    }
}

AppNode* create_AppNode(const char* id, MRNode* regions) {
    AppNode* newNode = (AppNode*)malloc(sizeof(AppNode));
    if (!newNode) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    strcpy(newNode->id, id);
    newNode->regions = regions;
    newNode->next = NULL;
    return newNode;
}

void append_AppNode(AppNode* head, const char* id, MRNode* regions, pthread_mutex_t mutex) {
    AppNode* newNode = create_AppNode(id, regions);

    pthread_mutex_lock(&mutex);
    if (!head) {
        head = newNode;
    } else {
        AppNode* current = head;
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
    pthread_mutex_unlock(&mutex);
}

void print_list(AppNode* head) {
    AppNode* current = head;
    while (current) {
        printf("ID: %s -> ", current->id);
        print_regions(current->regions);
        current = current->next;
    }
    printf("NULL\n");
}

void free_list(AppNode* head) {
    AppNode* current = head;
    while (current) {
        AppNode* temp = current;
        current = current->next;
        free_regions(temp->regions);
        free(temp->id);
        free(temp);
    }
}

MRNode* get_regions(AppNode* head, const char* id) {
    AppNode* current = atomic_load(&head);
    while (current) {
        if (strcmp(current->id, id) == 0) {
            return current->regions;
        }
        current = atomic_load(&current->next);
    }
    return NULL;
}

int remove_MRNode(AppNode* head, const char* id, void* address) {
    AppNode* current = atomic_load(&head);
    
    while (current) {
        if (strcmp(current->id, id) == 0) {
            MRNode* prev = NULL;
            MRNode* currRegion = current->regions;
            
            while (currRegion) {
                if (currRegion->region.address == address) {
                    if (prev) {
                        prev->next = currRegion->next;
                    } else {
                        current->regions = currRegion->next;
                    }
                    free(currRegion);
                    return 1;
                }
                prev = currRegion;
                currRegion = currRegion->next;
            }
            return 0;
        }
        current = atomic_load(&current->next);
    }
    return 0;
}

int insert_MRNode(AppNode* head, const char* id, void* address, size_t size, int flags) {
    AppNode* current = head;
    
    while (current) {
        if (strcmp(current->id, id) == 0) {
            MRNode* newRegion = (MRNode*)malloc(sizeof(MRNode));
            if (!newRegion) {
                perror("Failed to allocate memory for MRNode");
                return 0;
            }
            newRegion->region.address = address;
            newRegion->region.size = size;
            newRegion->region.flags = flags;
            newRegion->next = NULL;
            
            if (current->regions == NULL) {
                current->regions = newRegion;
            } else {
                MRNode* last = current->regions;
                while (last->next != NULL) {
                    last = last->next;
                }
                last->next = newRegion;
            }
            
            return 1;
        }
        current = current->next;
    }
    return 0;
}