#include "memory_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static HashTable *hashTable = NULL;

unsigned int hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash % hashTable->size;
}

unsigned int hash_int(int num) {
    unsigned long hash = (num * 2654435761);
    return (unsigned int)(hash & (hashTable->size - 1));
}

Bucket *get_bucket_from_name(const char *functionName) {
    Bucket *currentBucket = hashTable->buckets[hash_string(functionName)];
    while (currentBucket != NULL &&
        currentBucket->functionName != NULL &&
        strcmp(currentBucket->functionName, functionName) != 0)
    {
        currentBucket = currentBucket->next;
    }
    return currentBucket;
}

Bucket *get_bucket_from_tid(pid_t tid) {
    Bucket *currentBucket = hashTable->buckets[hash_int(tid)];
    while (currentBucket != NULL && currentBucket->tid != tid) {
        currentBucket = currentBucket->next;
    }
    return currentBucket;
}

void insert_app_function(pid_t tid, const char *functionName, IsolateFunction *function) {
    Bucket **head;
    Bucket *currentBucket;
    unsigned int bucket_ind;

    if (tid) {
        bucket_ind = hash_int(tid);
        currentBucket = get_bucket_from_tid(tid);
    } else {
        bucket_ind = hash_string(functionName);
        currentBucket = get_bucket_from_name(functionName);
    }

    head = &hashTable->buckets[bucket_ind];
    if (currentBucket == NULL) {
        currentBucket = (Bucket *)malloc(sizeof(Bucket));
        if (functionName) {
            currentBucket->functionName = strdup(functionName);
        }
        currentBucket->function = function;
        currentBucket->next = *head;
        currentBucket->tid = tid;
        *head = currentBucket;
    }
}

void remove_app_function(pid_t tid, const char *functionName) {
    unsigned int bucket_ind = tid ? hash_int(tid) : hash_string(functionName);
    Bucket **head = &hashTable->buckets[bucket_ind];
    Bucket *currentBucket = *head;
    Bucket *previousBucket = NULL;
    while (currentBucket != NULL) {
        if ((currentBucket->functionName == NULL && currentBucket->tid == 0)
            || (currentBucket->tid != tid && strcmp(currentBucket->functionName, functionName) != 0))
        {
            previousBucket = currentBucket;
            currentBucket = currentBucket->next;
            continue;
        } else if (currentBucket == *head) {
            *head = currentBucket->next;
        } else {
            previousBucket->next = currentBucket->next;
        }
        destroy_isolate_function(currentBucket->function);
        if (currentBucket->functionName)
            free(currentBucket->functionName);
        free(currentBucket);
        break;
    }
}

IsolateFunction *get_app_function(pid_t tid, const char *functionName) {
    Bucket *currentBucket = tid ? get_bucket_from_tid(tid) : get_bucket_from_name(functionName);
    return currentBucket ? currentBucket->function : NULL;
}

Function to create a hash table
void init_hash_table(int size) {
    hashTable = (HashTable *)malloc(sizeof(HashTable));
    if (!hashTable) {
        fprintf(stderr, "Could not allocate hashTable\n");
        exit(1);
    }

    int i;
    float aux = (float)size;
    for (i = 1; aux > 2; i++) {
        aux /= 2;
    }
    size = 1 << i;

    hashTable->size = size;
    hashTable->buckets = (Bucket **)calloc(size, sizeof(Bucket));
    if (!hashTable->buckets) {
        free(hashTable);
        exit(1);
    }
}

void free_hash_table() {
    for (int i = 0; i < hashTable->size; i++) {
        Bucket *currentBucket = hashTable->buckets[i];
        Bucket *nextBucket;

        while (currentBucket != NULL) {
            nextBucket = currentBucket->next;
            destroy_isolate_function(currentBucket->function);
            if (currentBucket->functionName)
                free(currentBucket->functionName);
            free(currentBucket);
            currentBucket = nextBucket;
        }
    }

    free(hashTable->buckets);
    free(hashTable);
}