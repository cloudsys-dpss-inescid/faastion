#ifndef THREADMAP_H
#define THREADMAP_H

#include <pthread.h>

#define TABLE_SIZE 16

typedef struct ThreadNode {
    int nthreads;
    pthread_mutex_t mutex;
} ThreadNode;

typedef struct ThreadMap {
    ThreadNode* buckets[TABLE_SIZE];
} ThreadMap;

ThreadNode* create_thread_node();
void init_thread_map(ThreadMap* map);
void insert_thread(ThreadMap* map, int domain);
void remove_thread(ThreadMap* map, int domain);

#endif
