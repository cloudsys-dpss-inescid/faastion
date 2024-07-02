#ifndef __UTILS__MALLOC_H
#define __UTILS__MALLOC_H

#include <stdlib.h>

#define CHUNK_SIZE (10*1024*1024)

typedef struct {
    double (*function)(size_t length);
    char *name;
} test_malloc_t;

test_malloc_t find_malloc_t(test_malloc_t functions[], int n, const char* name);

void malloc_bench_harness(test_malloc_t func, int n, int length);
double malloc_init(size_t length);
double malloc_no_init(size_t length);
double malloc_init_no_free(size_t length);
double malloc_no_init_no_free(size_t length);

void alloc_chunks(void *chunks[], int n);
void free_chunks(void *chunks[], int n);
void make_fragments(void *chunks[], int n, int fragment_count);
void write_chunk(void* addr, size_t size);

#endif