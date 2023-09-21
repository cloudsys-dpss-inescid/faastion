#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "threadmap.h"

void
init_thread_map(ThreadMap* map)
{
    for (int i = 0; i < TABLE_SIZE; i++) {
        map->buckets[i] = create_thread_node();
    }
}

ThreadNode*
create_thread_node()
{
    ThreadNode* newNode = (ThreadNode*)malloc(sizeof(ThreadNode));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    
    newNode->nthreads = 0;
    pthread_mutex_init(&(newNode->mutex), NULL);

    return newNode;
}

void
insert_thread(ThreadMap* map, int domain)
{
    ThreadNode* node = map->buckets[domain];
    
    pthread_mutex_lock(&(node->mutex));
    node->nthreads++;
    pthread_mutex_unlock(&(node->mutex));
}

void
remove_thread(ThreadMap* map, int domain)
{
    ThreadNode* node = map->buckets[domain];

    pthread_mutex_lock(&(node->mutex));
    node->nthreads--;
    pthread_mutex_unlock(&(node->mutex));
}
