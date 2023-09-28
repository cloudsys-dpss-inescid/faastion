#include "../utils/appmap.h"
#include <semaphore.h>

enum Status {
    IN_PROGRESS = 0,
    DONE = 1
};

struct Supervisor {
    sem_t perms;
    sem_t filter;
    enum Status status;
    int fd;
    char* app;
};

/* Semaphore synchronization */
void signal_semaphore(sem_t* semaphore);
void wait_semaphore(sem_t* semaphore);

/* Lazy loading */
void init_app_array(char* array[]);

/* Seccomp */
void init_supervisors(struct Supervisor array[]);

/* Preload */
char* extract_basename(const char* filePath);
void get_memory_regions(AppMap* map, char* id, const char* path);
