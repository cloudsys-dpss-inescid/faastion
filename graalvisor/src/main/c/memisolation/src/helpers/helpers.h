#include "../utils/appmap.h"
#include <semaphore.h>
#include <pthread.h>

#define NUM_PROCESSES 20

enum Status {
    ACTIVE = 0,
    DONE = 1,
};

enum Flag {
    NATIVE = 0,
    MANAGED = 1
};

struct Supervisor {
    enum Flag execution;
    enum Status status;
    int fd;
    char app[256];
};


/* Seccomp */
void init_supervisors(struct Supervisor supervisors[], int size);

/* Thread Count */
void init_thread_count(int threadCount[], int size);

/* Eager/Lazy setting */
void init_cache(char* cache[], int size);

/* Preload */
char* extract_basename(const char* filePath);
void get_memory_regions(AppNode* apps, char* id, const char* path, pthread_mutex_t mutex);
