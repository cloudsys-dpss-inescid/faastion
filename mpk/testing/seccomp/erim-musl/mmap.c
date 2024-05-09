#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>

void * doMmap() {
    void *mapped_mem = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (mapped_mem == MAP_FAILED)
        perror("mmap");
    else
        printf("[MT]: SUCCESS: mmap() returned %p\n", mapped_mem);
    
    return mapped_mem;
}