#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "appmap.h"

void initAppMap(AppMap* map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->buckets[i] = NULL;
    }
    pthread_mutex_init(&(map->mutex), NULL);
}

unsigned long hash_string(const char* key) {
    unsigned long hashValue = 14695981039346656037UL;
    const unsigned char* str = (const unsigned char*)key;

    while (*str) {
        hashValue ^= *str++;
        hashValue *= 1099511628211UL;
    }

    return hashValue % TABLE_SIZE;
}

AppNode* createAppNode(char* id, MemoryRegion memReg) {
    AppNode* newNode = (AppNode*)malloc(sizeof(AppNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    strcpy(newNode->id, id);
    newNode->memReg = memReg;
    newNode->next = NULL;
    return newNode;
}

void insertApp(AppMap* map, char* id, MemoryRegion memReg) {
    unsigned long index = hash_string((const char*)id);
    AppNode* newNode = createAppNode(id, memReg);
    
    pthread_mutex_lock(&(map->mutex));

    if (map->buckets[index] == NULL) {
        map->buckets[index] = newNode;
    } else {
        AppNode* currentNode = map->buckets[index];
        while (currentNode->next != NULL) {
            currentNode = currentNode->next;
        }
        currentNode->next = newNode;
    }

    pthread_mutex_unlock(&(map->mutex));
}

MemoryRegion* getRegions(AppMap map, char* id, size_t* count) {
    unsigned long index = hash_string((const char*)id);
    AppNode* currentNode = map.buckets[index];
    MemoryRegion* values = NULL;
    size_t numValues = 0;

    pthread_mutex_lock(&map.mutex);

    while (currentNode != NULL) {
        if (!strcmp(currentNode->id, id)) {
            // Add the value to the dynamic array
            values = (MemoryRegion*)realloc(values, (numValues + 1) * sizeof(MemoryRegion));
            values[numValues] = currentNode->memReg;
            ++numValues;
        }
        currentNode = currentNode->next;
    }

    pthread_mutex_unlock(&map.mutex);

    *count = numValues;

    return values;
}

void printAppMap(AppMap map, int verbose) {
    if (!verbose)
        return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        AppNode* currentNode = map.buckets[i];
        while (currentNode != NULL) {
            fprintf(stderr, "%s: (%p, %ld)\n", currentNode->id, currentNode->memReg.address, currentNode->memReg.size);
            currentNode = currentNode->next;
        }
    }
}
