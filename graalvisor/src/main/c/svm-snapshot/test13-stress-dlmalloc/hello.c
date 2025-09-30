#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <unistd.h>
#include "../graal_isolate.h"
#include <sys/mman.h>
#include <errno.h>

/* This test is for stress testing our custom memory allocator, to try to find all possible kinds of errors from a memory allocator. */

#define NUM_THREADS 16
#define OPS_PER_THREAD 100
#define NUM_OP_TYPES 4 // malloc, free, calloc, realloc

// Mutex to protect shared resources (like stdout)
pthread_mutex_t io_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int thread_id;
    int ops_count[NUM_OP_TYPES];
} ThreadData;

void* thread_func(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int thread_id = data->thread_id;
    
    // Seed random number generator for this thread
    unsigned int seed = time(NULL) ^ thread_id;
    
    // Array to track allocations for realloc/free
    void* allocations[OPS_PER_THREAD] = {NULL};
    int alloc_index = 0;
    
    for (int i = 0; i < OPS_PER_THREAD; i++) {
        int op_type = rand_r(&seed) % NUM_OP_TYPES;
        data->ops_count[op_type]++;
        
        switch(op_type) {
            case 0: { // malloc
                size_t size = (rand_r(&seed) % 4096) + 1;
                void* ptr = malloc(size);
                if (ptr && alloc_index < OPS_PER_THREAD) {
                    allocations[alloc_index++] = ptr;
                }
                break;
            }
            case 1: { // free
                if (alloc_index > 0) {
                    int idx = rand_r(&seed) % alloc_index;
                    free(allocations[idx]);
                    allocations[idx] = allocations[--alloc_index];
                }
                break;
            }
            case 2: { // calloc
                size_t num = (rand_r(&seed) % 16) + 1;
                size_t size = (rand_r(&seed) % 256) + 1;
                void* ptr = calloc(num, size);
                if (ptr && alloc_index < OPS_PER_THREAD) {
                    allocations[alloc_index++] = ptr;
                }
                break;
            }
            case 3: { // realloc
                if (alloc_index > 0) {
                    int idx = rand_r(&seed) % alloc_index;
                    size_t new_size = (rand_r(&seed) % 4096) + 1;
                    void* new_ptr = realloc(allocations[idx], new_size);
                    if (new_ptr) {
                        allocations[idx] = new_ptr;
                    }
                }
                break;
            }
        }
    }
    
    // Clean up any remaining allocations
    for (int i = 0; i < alloc_index; i++) {
        free(allocations[i]);
    }
    
    // Print thread summary
    pthread_mutex_lock(&io_mutex);
    printf("Thread %2d: malloc=%d, free=%d, calloc=%d, realloc=%d\n",
           thread_id, 
           data->ops_count[0], data->ops_count[1],
           data->ops_count[2], data->ops_count[3]);
    pthread_mutex_unlock(&io_mutex);
    
    return NULL;
}

int graal_create_isolate (graal_create_isolate_params_t* params, graal_isolate_t** isolate, graal_isolatethread_t** thread) {
	    *thread = NULL;
	        *isolate = NULL;
		    return 0;
}

int graal_tear_down_isolate(graal_isolatethread_t* thread) {
	    return 0;
}

void entrypoint(graal_isolatethread_t* thread, const char* fin, const char* fout, unsigned long fout_len) {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    
    // Initialize thread data
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        for (int j = 0; j < NUM_OP_TYPES; j++) {
            thread_data[i].ops_count[j] = 0;
        }
    }
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_func, &thread_data[i]);
    }
    
    // Wait for threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print summary
    int total_ops[NUM_OP_TYPES] = {0};
    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < NUM_OP_TYPES; j++) {
            total_ops[j] += thread_data[i].ops_count[j];
        }
    }
    
    printf("\nTotal operations across all threads:\n");
    printf("malloc: %d\nfree: %d\ncalloc: %d\nrealloc: %d\n",
           total_ops[0], total_ops[1], total_ops[2], total_ops[3]);
    
    pthread_mutex_destroy(&io_mutex);
}

int graal_detach_thread(graal_isolatethread_t* thread) {
	    return 0;
}

int graal_attach_thread(graal_isolate_t* isolate, graal_isolatethread_t** thread) {
	    *thread = NULL;
	        return 0;
}
