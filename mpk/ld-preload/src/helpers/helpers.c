#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "helpers.h"

/* Auxiliary functions */
void
init_supervisors(struct Supervisor array[], int size)
{
    for (int i = 0; i < size; i++) {
        sem_init(&array[i].set, 0, 0);
        sem_init(&array[i].filter, 0, 0);
        sem_init(&array[i].perms, 0, 0);
        sem_init(&array[i].handler, 0, 0);
        strcpy(array[i].app, "");
        array[i].status = IN_PROGRESS;
        array[i].fd = 0;
    }
}

void
init_cache_array(struct CacheApp cache[], int size)
{
    for (int i = 0; i < size; i++) {
        strcpy(cache[i].app, "");
        cache[i].value = 1;
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
