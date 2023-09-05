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

ThreadNode* createThreadNode(pthread_t threadId) {
    ThreadNode* newNode = (ThreadNode*)malloc(sizeof(ThreadNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    newNode->threadId = threadId;
    newNode->next = NULL;
    return newNode;
}

void insertThread(ThreadMap* map, int domain, pthread_t threadId) {
    ThreadNode* newNode = createThreadNode(threadId);
    
    pthread_mutex_lock(&(map->mutex));

    if (map->buckets[domain] == NULL) {
        map->buckets[domain] = newNode;
    } else {
        ThreadNode* currentNode = map->buckets[domain];
        while (currentNode->next != NULL) {
            currentNode = currentNode->next;
        }
        currentNode->next = newNode;
    }

    pthread_mutex_unlock(&(map->mutex));
}

void removeThread(ThreadMap* map, int domain, pthread_t threadId) {
    pthread_mutex_lock(&(map->mutex));

    ThreadNode* currentNode = map->buckets[domain];
    ThreadNode* prevNode = NULL;

    while (currentNode != NULL) {
        if (currentNode->threadId == threadId) {
            if (prevNode == NULL) {
                map->buckets[domain] = currentNode->next;
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
