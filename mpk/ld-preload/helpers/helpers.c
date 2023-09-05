#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "helpers.h"

/* Auxiliary functions */
void initAppArray(char** appIds) {
    for (size_t i = 0; i < 16; i++) {
        appIds[i] = NULL;
    }
}

char* extractBaseName(const char* filePath) {
    char* baseName = strrchr(filePath, '/');
    if (baseName != NULL) {
        return baseName + 1;
    }
    return (char*)filePath;
}

void getMemoryRegions(AppMap* map, char* id, const char* path) {
    const char* libraryName = extractBaseName(path);

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

        insertApp(map, id, memReg);
    }

    fclose(mapsFile);
}
