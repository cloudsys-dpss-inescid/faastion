#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>

void * doMmap() {
    void *mapped_mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (mapped_mem == MAP_FAILED)
        perror("mmap");
    
    return mapped_mem;
}