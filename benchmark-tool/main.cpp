#include <iostream>
#include <map>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include "utils/timer.h"
#include "utils/operations.h"

#define ROUNDS 100

std::map<int, long*> my_map;
int numberThreads = 0;
int numberPages = 0;
void *(*option)(void*) = NULL;


void* domain(void *args)
{
    long* my_times = (long*)malloc(ROUNDS * sizeof(long));
    TIMER startTime, stopTime, temp;
    int* buffer = ((struct arguments*)args)->buffer;
    int pkey = ((struct arguments*)args)->pkey;

    if (pkey < 0) {
        errExit("pkey < 0");
    }

    my_map[pthread_self()] = my_times;

    for (int i = 0; i < ROUNDS; i++) {
        startTime = read_time(temp);
        /*
         * Set the protection key on "buffer".
         */
        if (pkey_mprotect(buffer, getpagesize(), PROT_READ | PROT_WRITE, pkey) == -1) {
            errExit("pkey_mprotect");
        }

        stopTime = read_time(temp);
        my_times[i] = time_diff(startTime, stopTime);
    }

    return NULL;
}

void* access(void *args)
{
    long* my_times = (long*)malloc(ROUNDS * sizeof(long));
    TIMER startTime, stopTime, temp;
    int pkey = ((struct arguments*)args)->pkey;

    if (pkey < 0) {
        errExit("pkey < 0");
    }

    my_map[pthread_self()] = my_times;

    for (int i = 0; i < ROUNDS; i++) {
        startTime = read_time(temp);

        /*
         * Enable/Disable access to any memory with "pkey" set.
         */
        if (pkey_set(pkey, 0) == -1) {
            errExit("pkey_set");
        }

        stopTime = read_time(temp);
        my_times[i] = time_diff(startTime, stopTime);
    }

    return NULL;
}

void run_threads()
{
    pthread_t * slaves = (pthread_t*) malloc(sizeof(pthread_t)*numberThreads);

    arguments *args = get_thread_args(numberPages);

    for (int i = 0; i < numberThreads; i++) {
        if (pthread_create(&slaves[i], NULL, option, (void *)args) != 0){
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

void print_benchmark_results()
{
    float sum = 0;

    for (const auto &ele : my_map) {
        long* times = (long*)ele.second;
        for (int i = 0; i < ROUNDS; i++) {
            fprintf(stdout, "%d\n", times[i]);
        }
    }
}

void parse_args (int argc, char* argv[])
{
    if (argc != 4)
        errExit("Invalid format\n");

    numberThreads = atoi(argv[2]);
    numberPages = atoi(argv[3]);

    if (numberThreads <= 0)
        errExit("Invalid number of threads\n");
    if (numberPages <= 0)
        errExit("Invalid number of pages\n");
    if (!strcmp(argv[1],"domain"))
        option = &domain;
    else if (!strcmp(argv[1],"access"))
        option = &access;
    else
        errExit("Invalid option\n");
}

int main(int argc, char* argv[])
{
    /* initial arguments */
    parse_args(argc, argv);

    /* create and run threads */
    run_threads();

    print_benchmark_results();

    exit(EXIT_SUCCESS);
}
