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

std::map<int, float> my_map;
int numberThreads = 0;
int numberPages = 0;
void *(*option)(void*) = NULL;


void* domain(void *args)
{
    TIMER startTime, stopTime, temp;

    int* buffer = ((struct arguments*)args)->buffer;
    int pkey = ((struct arguments*)args)->pkey;

    startTime = read_time(temp);
    /*
     * Set the protection key on "buffer".
     */
    if (pkey_mprotect(buffer, getpagesize(),
                            PROT_READ | PROT_WRITE, pkey) == -1)
        errExit("pkey_mprotect");

    stopTime = read_time(temp);
    my_map[pthread_self()] = time_diff(startTime, stopTime);

    return NULL;
}

void* access(void *args)
{   
    TIMER startTime, stopTime, temp;
    int pkey = ((struct arguments*)args)->pkey;

    startTime = read_time(temp);
    /*
     * Enable/Disable access to any memory with "pkey" set.
     */
    if (pkey >= 0 && pkey_set(pkey, 0) == -1)
        errExit("pkey_set");
    
    stopTime = read_time(temp);
    my_map[pthread_self()] = time_diff(startTime, stopTime);

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
        sum += ele.second;
    }
    float average = static_cast<float>(sum) / my_map.size();

    fprintf(stdout, "%f\n", average);
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