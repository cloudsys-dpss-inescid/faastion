#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void* fun(void * arg)
{
    // printing: printf, fprintf
    printf("Hello from printf!\n");
    fprintf(stderr, "Hello to stderr!\n");
    fprintf(stderr, "Function received: %s\n", (char*)arg);

    // memory allocation: mmap and munmap
    void *mapped_mem = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap");
    }
    if (munmap(mapped_mem, 4096)) {
        perror("munmap");
    }

    // file handling: fopen, fclose, fprintf.
    FILE* f = fopen("mmap.log", "w");
    fprintf(f, "Hello\n");
    fclose(f);

    // syscall: sleep, write
    sleep(1);
    write(1, "Hello!\n", 7);
    syscall(SYS_write, 1, "Hello!\n", 7);

    // memory allocation 2: malloc, free
    int* malloced_mem = (int*) malloc(32 * sizeof(int*));
    malloced_mem[0] = 0;
    free(malloced_mem);

    return arg;
}