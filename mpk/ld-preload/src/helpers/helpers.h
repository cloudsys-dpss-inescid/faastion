#include "../utils/appmap.h"
#include <semaphore.h>

struct NotifyFileDescriptor {
    sem_t semaphore;
    int fd;
};

/* Notify file descriptors (Seccomp) */
void signal_semaphore(struct NotifyFileDescriptor* nfd);
void wait_semaphore(struct NotifyFileDescriptor* nfd);
void init_notify_array(struct NotifyFileDescriptor array[]);

/* Lazy loading */
void init_app_array(char* array[]);

/* Preload */
char* extract_basename(const char* filePath);
void get_memory_regions(AppMap* map, char* id, const char* path);
