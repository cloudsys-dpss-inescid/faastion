#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "thread_map.h"

static ThreadMap threadMap;

void init_thread_map(int size) {
    threadMap.size = size;
    threadMap.buckets = (ThreadInfo *)calloc(sizeof(ThreadInfo), size);
    if (threadMap.buckets == NULL) {
        fprintf(stderr, "Could not allocate threadMap\n");
        exit(1);
    }
}

ThreadInfo *get_thread_info(pid_t tid) {
    return &threadMap.buckets[tid % threadMap.size];
}

ThreadInfo *insert_thread_info(pid_t tid, const char *appName) {
    ThreadInfo *info = &threadMap.buckets[tid % threadMap.size];
    info->tid = tid;
    info->register_mmaps = 1;
    info->appName = strdup(appName);
    return info;
}

void remove_thread_info(pid_t tid) {
    ThreadInfo *info = &threadMap.buckets[tid % threadMap.size];
    free(info->appName);
    memset(info, 0, sizeof(ThreadInfo));
}

void free_thread_map() {
    for (int i = 0; i < threadMap.size; i++) {
        if (threadMap.buckets[i].appName != NULL) {
            free(threadMap.buckets[i].appName);  // Free any allocated appName strings
        }
    }
    free(threadMap.buckets);  // Free the entire buckets array
    threadMap.buckets = NULL;
    threadMap.size = 0;
}