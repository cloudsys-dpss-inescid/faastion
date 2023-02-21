#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "timer.h"

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                          } while (0)

int numberThreads = 0;

struct arguments {
    int* buffer;
    int pkey;
};

void parseArgs (int argc, char* argv[])
{
    if (argc != 2)
        errExit("Invalid format\n");
    
    numberThreads = atoi(argv[1]);

    if (numberThreads <= 0 || numberThreads > 15)
        errExit("Invalid number of threads\n");
}

int* memAlloc()
{
    int *buffer;

    /*
     * Allocate one page of memory.
     */
    buffer = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (buffer == MAP_FAILED)
        errExit("mmap");

    return buffer;   
}

int pkeyAlloc()
{
    int pkey;

    /*
     * Allocate a protection key:
     */
    pkey = pkey_alloc(0, PKEY_DISABLE_ACCESS);
    if (pkey == -1)
        errExit("pkey_alloc");
    
    return pkey;
}

void* protect(void *args)
{
    TIMER startTime, stopTime, temp;
    int* buffer = ((struct arguments*)args)->buffer;
    int pkey = ((struct arguments*)args)->pkey;

    /*
     * Set the protection key on "buffer".
     */
    startTime = read_time(temp);
    if (pkey_mprotect(buffer, getpagesize(),
                            PROT_READ | PROT_WRITE, pkey) == -1)
        errExit("pkey_mprotect");
    stopTime = read_time(temp);
    fprintf(stdout, "[%ld] Domain change completed in %.8f seconds.\n", pthread_self(), time_diff(startTime, stopTime));

}

void runThreads(struct arguments *args) 
{
    pthread_t * slaves = (pthread_t*) malloc(sizeof(pthread_t)*numberThreads);

    for (int i = 0; i < numberThreads; i++) {
        if (pthread_create(&slaves[i], NULL, protect, (void *)args) != 0){
            perror("Can't create thread\n");
            errExit("pthread_create");
        }
    }

    for(int i = 0; i < numberThreads; i++) {
        if(pthread_join(slaves[i], NULL)) {
            perror("Thread can't join\n");
            errExit("pthread_join");
        }
    }
    free(slaves);
    
}

int main(int argc, char* argv[]) 
{
    /* initial arguments */
    parseArgs(argc, argv);

    struct arguments *args = (struct arguments *)malloc(sizeof(struct arguments));
    args->buffer = memAlloc();
    args->pkey = pkeyAlloc();
    
    /* create and run threads */
    runThreads(args);

    exit(EXIT_SUCCESS);
}