#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "threadmap.h"

void initThreadMap(ThreadMap* map) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->buckets[i] = createThreadNode();
    }
}

ThreadNode* createThreadNode() {
    ThreadNode* newNode = (ThreadNode*)malloc(sizeof(ThreadNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    newNode->nthreads = 0;
    pthread_mutex_init(&(newNode->mutex), NULL);

    return newNode;
}

void insertThread(ThreadMap* map, int domain) {
    ThreadNode* node = map->buckets[domain];
    
    pthread_mutex_lock(&(node->mutex));
    node->nthreads++;
    pthread_mutex_unlock(&(node->mutex));
}

void removeThread(ThreadMap* map, int domain) {
    ThreadNode* node = map->buckets[domain];

    pthread_mutex_lock(&(node->mutex));
    node->nthreads--;
    pthread_mutex_unlock(&(node->mutex));
}
