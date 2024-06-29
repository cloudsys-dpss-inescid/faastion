#ifndef APPMAP_H
#define APPMAP_H

#include <dlfcn.h>
#include <pthread.h>

typedef struct {
    void* address;
    size_t size;
    int flags;
} MemoryRegion;

typedef struct MRNode {
    MemoryRegion region;
    struct MRNode* next;
} MRNode;

typedef struct AppNode {
    char id[256];
    struct AppNode* next;
    struct MRNode* regions;
} AppNode;

MRNode* create_MRNode(void* address, size_t size, int flags);
void append_MRNode(MRNode* head, void* address, size_t size, int flags);
void print_regions(MRNode* head);
void free_regions(MRNode* head);

AppNode* create_AppNode(const char* id, MRNode* regions);
void append_AppNode(AppNode* head, const char* id, MRNode* regions, pthread_mutex_t mutex);
void print_list(AppNode* head);
void free_list(AppNode* head);
MRNode* get_regions(AppNode* head, const char* id);
int remove_MRNode(AppNode* head, const char* id, void* address);
int insert_MRNode(AppNode* head, const char* id, void* address, size_t size, int flags);

#endif
