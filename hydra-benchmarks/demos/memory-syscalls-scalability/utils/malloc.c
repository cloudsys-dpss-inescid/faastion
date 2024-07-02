#include "malloc.h"
#include "utils.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

test_malloc_t find_malloc_t(test_malloc_t test_functions[], int n, const char* test_name) 
{
    test_malloc_t test = { NULL, "unknown" };
    for (int i = 0; i < n; i++) {
        if (strcmp(test_functions[i].name, test_name) == 0) {
            test = test_functions[i];
        }
    }

    assert_that(test.function != NULL, "failed to locate test function with such name");
    return test;
}

void malloc_bench_harness(test_malloc_t func, int n, int length)
{
    double accum = 0;

    for (int i = 0; i < n; i++) {
        double t = func.function(length); 
        printf("%.9lf\n", t);
        fflush(stdout);
        accum += t;
    }

    double avg = accum / n;
    UTIL_LOGF("avg: %.9lf", avg);
}

double malloc_init(size_t length)
{
    UTIL_CLOCK_INIT

    UTIL_CLOCK_START;
        void *mem = malloc(length);
        assert_that(mem != NULL, "malloc failed");
        for(size_t i = 0; i < length; i++) {
            ((char *)mem)[i] = 0;
        }
    UTIL_CLOCK_STOP;
    free(mem);

    return UTIL_CLOCK_DELTA;
}

double malloc_no_init(size_t length)
{
    UTIL_CLOCK_INIT

    UTIL_CLOCK_START;
        void *mem = malloc(length);
        assert_that(mem != NULL, "malloc failed");
    UTIL_CLOCK_STOP;
    free(mem);

    return nanos(start, stop);
}

double malloc_init_no_free(size_t length) 
{
    UTIL_CLOCK_INIT
    
    UTIL_CLOCK_START;
        void *mem = malloc(length);
        assert_that(mem != NULL, "malloc failed");
        for(size_t i = 0; i < length; i++) {
            ((char *)mem)[i] = 0;
        }
    UTIL_CLOCK_STOP;

    return nanos(start, stop);
}

double malloc_no_init_no_free(size_t length) 
{
    UTIL_CLOCK_INIT
    
    UTIL_CLOCK_START;
        void *mem = malloc(length);
        assert_that(mem != NULL, "malloc failed");
    UTIL_CLOCK_STOP;

    return nanos(start, stop);
}

void write_chunk(void* addr, size_t size) 
{
    for (size_t i = 0; i < size; i += 16) {
        *((char *)(addr + i)) = (char)i;
    }
}

void alloc_chunks(void *chunks[], int n)
{
    srand(time(NULL));

    unsigned long alloc_size = 0;
    for (int i = 0; i < n; i++) {
        chunks[i] = malloc(CHUNK_SIZE);
        assert_that(chunks[i] != NULL, "map chunk failed");
        write_chunk(chunks[i], CHUNK_SIZE);
        alloc_size += CHUNK_SIZE;
    }

    UTIL_LOGF("allocated %d chunks, size %ld", n, alloc_size);
}

void make_fragments(void *chunks[], int n, int fragment_count)
{
    int step = n / fragment_count;
    for (int i = 0; i < fragment_count; i += step) {
        free(chunks[i]);
        chunks[i] = NULL;
    }
}

void free_chunks(void *chunks[], int n)
{
    if (chunks == NULL) {
        return;
    }

    for (int i = 0; i < n; i++) {
        if (chunks[i] != NULL) {
            free(chunks[i]);
            chunks[i] = NULL;
        }
    }
    free(chunks);
}
