#ifndef __THREAD_MAP_H__
#define __THREAD_MAP_H__

#include <unistd.h>

typedef struct {
    pid_t tid;
    char register_mmaps;
    char *appName;
} ThreadInfo;

typedef struct {
    int size;
    ThreadInfo *buckets;
} ThreadMap;

void init_thread_map(int size);

ThreadInfo *insert_thread_info(pid_t tid, const char *appName);

ThreadInfo *get_thread_info(pid_t tid);

void remove_thread_info(pid_t tid);

void free_thread_map();

#endif // __THREAD_MAP_H__