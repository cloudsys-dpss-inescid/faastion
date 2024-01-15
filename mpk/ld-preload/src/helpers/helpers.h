#include "../utils/appmap.h"
#include <semaphore.h>

enum Status {
    IN_PROGRESS = 0,
    DONE = 1
};

struct Supervisor {
    sem_t filter;
    sem_t perms;
    sem_t set;
    sem_t handler;
    enum Status status;
    int fd;
    char app[33];
};

struct CacheApp {
    char app[33];
    int value;
};

/* Eager/Lazy setting */
void init_cache_array(struct CacheApp cache[], int size);

/* Seccomp */
void init_supervisors(struct Supervisor array[], int size);

/* Preload */
char* extract_basename(const char* filePath);
void get_memory_regions(AppMap* map, char* id, const char* path);
