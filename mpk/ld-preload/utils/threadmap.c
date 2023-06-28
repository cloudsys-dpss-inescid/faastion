#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "threadmap.h"

void initThreadMap(ThreadMap* map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->buckets[i] = NULL;
    }
    pthread_mutex_init(&(map->mutex), NULL);
}

unsigned long hash_int(int key) {
    unsigned long hashValue = 14695981039346656037UL;  // FNV offset basis
    const unsigned char* p = (const unsigned char*)&key;
    size_t keySize = sizeof(int);

    for (size_t i = 0; i < keySize; i++) {
        hashValue ^= p[i];
        hashValue *= 1099511628211UL;  // FNV prime
    }

    return hashValue % TABLE_SIZE;
}

ThreadNode* createThreadNode(int domain, pthread_t threadId) {
    ThreadNode* newNode = (ThreadNode*)malloc(sizeof(ThreadNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    newNode->domain = domain;
    newNode->threadId = threadId;
    newNode->next = NULL;
    return newNode;
}

void insertThread(ThreadMap* map, int domain, pthread_t threadId) {
    unsigned long index = hash_int(domain);
    ThreadNode* newNode = createThreadNode(domain, threadId);
    
    pthread_mutex_lock(&(map->mutex));

    if (map->buckets[index] == NULL) {
        map->buckets[index] = newNode;
    } else {
        ThreadNode* currentNode = map->buckets[index];
        while (currentNode->next != NULL) {
            currentNode = currentNode->next;
        }
        currentNode->next = newNode;
    }

    pthread_mutex_unlock(&(map->mutex));
}

void removeThread(ThreadMap* map, int domain, pthread_t threadId) {
    unsigned long index = hash_int(domain);

    pthread_mutex_lock(&(map->mutex));

    ThreadNode* currentNode = map->buckets[index];
    ThreadNode* prevNode = NULL;

    while (currentNode != NULL) {
        if (currentNode->domain == domain && currentNode->threadId == threadId) {
            if (prevNode == NULL) {
                map->buckets[index] = currentNode->next;
            } else {
                prevNode->next = currentNode->next;
            }
            free(currentNode);
            currentNode = NULL;
        } else {
            prevNode = currentNode;
            currentNode = currentNode->next;
        }
    }

    pthread_mutex_unlock(&(map->mutex));
}

void printThreadMap(ThreadMap map, int verbose) {
    if (!verbose)
        return;
    for (int i = 0; i < TABLE_SIZE; i++) {
        ThreadNode* currentNode = map.buckets[i];
        while (currentNode != NULL) {
            fprintf(stderr, "%d: (%ld)\n", currentNode->domain, currentNode->threadId);
            currentNode = currentNode->next;
        }
    }
}
