#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "helpers.h"

/* Auxiliary functions */
void
signal_semaphore(struct NotifyFileDescriptor* nfd)
{
    sem_post(&nfd->semaphore);
}

void
wait_semaphore(struct NotifyFileDescriptor* nfd)
{
    sem_wait(&nfd->semaphore);
}

void
init_notify_array(struct NotifyFileDescriptor array[])
{
    for (int i = 0; i < 16; i++) {
        sem_init(&array[i].semaphore, 0, 0);
        array[i].fd = 0;
    }
}

void
init_app_array(char* array[])
{
    for (size_t i = 0; i < 16; i++) {
        array[i] = NULL;
    }
}

char*
extract_basename(const char* filePath)
{
    char* baseName = strrchr(filePath, '/');
    if (baseName != NULL) {
        return baseName + 1;
    }
    return (char*)filePath;
}

void
get_memory_regions(AppMap* map, char* id, const char* path)
{
    const char* libraryName = extract_basename(path);

    FILE* mapsFile = fopen("/proc/self/maps", "r");
    if (!mapsFile) {
        fprintf(stderr, "Failed to open /proc/self/maps\n");
        exit(EXIT_FAILURE);
    }

    char line[256];
    MemoryRegion memReg;

    while (fgets(line, sizeof(line), mapsFile)) {
        if (strstr(line, libraryName) == NULL) {
            continue;
        }

        unsigned long startAddress, endAddress;
        sscanf(line, "%lx-%lx", &startAddress, &endAddress);

        memReg.address = (void*)startAddress;
        memReg.size = endAddress - startAddress;

        insert_app(map, id, memReg);
    }

    fclose(mapsFile);
}
