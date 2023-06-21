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

#define ROUNDS 1000
#define WARMUP 500

std::map<int, long*> my_map;
int numberThreads = 0;
int numberPages = 0;
void *(*option)(void*) = NULL;


void* domain(void *args)
{
    long* my_times = (long*)malloc(ROUNDS * sizeof(long));
    TIMER startTime, stopTime, temp;
    void* buffer = ((struct arguments*)args)->buffer;
    size_t buffer_size = ((struct arguments*)args)->buffer_size;
    int pkey = ((struct arguments*)args)->pkey;
    int pkey2 = ((struct arguments*)args)->pkey2;

    if (pkey < 0) {
        errExit("pkey < 0");
    }

    my_map[pthread_self()] = my_times;

    for (int i = 0; i < ROUNDS; i++) {
        startTime = read_time(temp);
        /*
         * Set the protection key on "buffer".
         */
        if (pkey_mprotect(buffer, buffer_size, PROT_READ, i % 2 ? pkey : pkey2)) {
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
    pthread_t * workers = (pthread_t*) malloc(sizeof(pthread_t)*numberThreads);

    for (int i = 0; i < numberThreads; i++) {
        if (pthread_create(&workers[i], NULL, option, (void *)get_thread_args(numberPages)) != 0){
            perror("Can't create thread\n");
            errExit("pthread_create");
        }
    }

    for(int i = 0; i < numberThreads; i++) {
        if(pthread_join(workers[i], NULL)) {
            perror("Thread can't join\n");
            errExit("pthread_join");
        }
    }
    free(workers);
}

void print_benchmark_results()
{
    float sum = 0;

    for (const auto &ele : my_map) {
        long* times = (long*)ele.second;
        for (int i = WARMUP; i < ROUNDS; i++) {
            fprintf(stdout, "%d\n", times[i]);
        }
    }
}

void parse_args (int argc, char* argv[])
{
    if (argc != 4) {
        errExit("Syntax: ./benchmark <mode> <number of threads> <number of pages>. Mode can be domain or access.\n");
    }

    numberThreads = atoi(argv[2]);
    numberPages = atoi(argv[3]);

    if (numberThreads <= 0) {
        errExit("Invalid number of threads\n");
    }

    if (numberPages <= 0) {
        errExit("Invalid number of pages\n");
    }

    if (!strcmp(argv[1],"domain")) {
        option = &domain;
    } else if (!strcmp(argv[1],"access")) {
        option = &access;
    } else {
        errExit("Invalid option\n");
    }
}

int main(int argc, char* argv[])
{
    parse_args(argc, argv);
    run_threads();
    print_benchmark_results();
    return 0;
}
