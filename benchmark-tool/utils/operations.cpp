#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include "operations.h"

arguments* get_thread_args(int numberPages)
{
    struct arguments *args = (struct arguments *)malloc(sizeof(struct arguments));

    args->buffer = mem_alloc(numberPages);
    args->pkey = key_alloc();

    return args;
}

int* mem_alloc(int numberPages)
{
    /*
     * Allocate one page of memory.
     */
    int *buffer = (int *)mmap(NULL, numberPages*getpagesize(), PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (buffer == MAP_FAILED)
        errExit("mmap");

    return buffer;   
}

int key_alloc()
{
    /*
     * Allocate a protection key:
     */
    int pkey = pkey_alloc(0, PKEY_DISABLE_ACCESS);
    if (pkey == -1)
        errExit("pkey_alloc");
    
    return pkey;
}