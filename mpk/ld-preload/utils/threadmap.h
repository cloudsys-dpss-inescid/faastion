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

void initThreadMap(ThreadMap* map);
ThreadNode* createThreadNode();
void insertThread(ThreadMap* map, int domain);
void removeThread(ThreadMap* map, int domain);

#endif
