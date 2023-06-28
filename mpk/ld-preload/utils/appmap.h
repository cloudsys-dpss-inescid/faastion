#ifndef APPMAP_H
#define APPMAP_H

#include <dlfcn.h>
#include <pthread.h>

#define TABLE_SIZE 16

typedef struct {
    void* address;
    size_t size;
} MemoryRegion;

typedef struct AppNode {
    char id[256];
    MemoryRegion memReg;
    struct AppNode* next;
} AppNode;

typedef struct AppMap {
    pthread_mutex_t mutex;
    AppNode* buckets[TABLE_SIZE];
} AppMap;

void initAppMap(AppMap* map);
unsigned long hash_str(const char *str);
AppNode* createAppNode(char* id, MemoryRegion memReg);
void insertApp(AppMap* map, char* id, MemoryRegion memReg);
MemoryRegion* getRegions(AppMap map, char* id, size_t* count);
void printAppMap(AppMap map, int verbose);

#endif
