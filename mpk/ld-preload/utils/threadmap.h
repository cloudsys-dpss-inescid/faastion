#ifndef THREADMAP_H
#define THREADMAP_H

#include <pthread.h>

#define TABLE_SIZE 16

typedef struct ThreadNode {
    pthread_t threadId;;
    struct ThreadNode* next;
} ThreadNode;

typedef struct ThreadMap {
    pthread_mutex_t mutex;
    ThreadNode* buckets[TABLE_SIZE];
} ThreadMap;

void initThreadMap(ThreadMap* map);
ThreadNode* createThreadNode(pthread_t threadId);
void insertThread(ThreadMap* map, int domain, pthread_t threadId);
void removeThread(ThreadMap* map, int domain, pthread_t threadId);

#endif
