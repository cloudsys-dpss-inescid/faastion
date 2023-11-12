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

/* Lazy loading */
void init_app_array(char* array[]);

/* Seccomp */
void init_supervisors(struct Supervisor array[]);

/* Preload */
char* extract_basename(const char* filePath);
void get_memory_regions(AppMap* map, char* id, char* libraryName);
