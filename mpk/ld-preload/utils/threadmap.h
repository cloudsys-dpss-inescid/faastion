#ifndef THREADMAP_H
#define THREADMAP_H

#include <pthread.h>

#define TABLE_SIZE 16

typedef struct ThreadNode {
    int domain;
    pthread_t threadId;;
    struct ThreadNode* next;
} ThreadNode;

typedef struct ThreadMap {
    pthread_mutex_t mutex;
    ThreadNode* buckets[TABLE_SIZE];
} ThreadMap;

void initThreadMap(ThreadMap* map);
unsigned long hash_int(int key);
ThreadNode* createThreadNode(int domain, pthread_t threadId);
void insertThread(ThreadMap* map, int domain, pthread_t threadId);
void removeThread(ThreadMap* map, int domain, pthread_t threadId);
void printThreadMap(ThreadMap map, int verbose);

#endif
