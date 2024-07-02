#include "../utils/utils.h"
#include "../utils/malloc.h"
#include <stdlib.h>
#include <sys/sysinfo.h>

void **chunks = NULL;
int chunk_count = 0;
int chunk_to_free = 0;

double free_one_chunk(size_t length)
{
    UNUSED(length);

    void *chunk = chunks[chunk_to_free];
    
    UTIL_CLOCK_INIT
    UTIL_CLOCK_START;
        free(chunk);
    UTIL_CLOCK_STOP;
    
    chunks[chunk_to_free] = NULL;
    chunk_to_free++;

    return nanos(start, stop);
}

double free_all_chunks(size_t length)
{
    UNUSED(length);
 
    UTIL_CLOCK_INIT
    UTIL_CLOCK_START;
        free_chunks(chunks, chunk_count);
    UTIL_CLOCK_STOP;
    
    chunks = NULL;
    return nanos(start, stop);
}

int main(int argc, char** argv) 
{
    assert_that(argc >= 3, "expected test_case_name and iter_count");

    test_malloc_t test_functions[] = {
        { free_one_chunk, "free_one_chunk" },
        { free_all_chunks, "free_all_chunks" },
    };

    test_malloc_t test = find_malloc_t(test_functions, sizeof(test_functions) / sizeof(test_functions[0]), argv[1]);

    int n = atoi(argv[2]);
    int size = ALLOC_SIZE;  
    if (argc > 3) {
        size = atoi(argv[3]);
    }

    UTIL_LOGF("config: %s , n=%d , size=%d", test.name, n, size);

    struct sysinfo info;
    sysinfo(&info);

    chunk_count = info.freeram / CHUNK_SIZE;
    UTIL_LOGF("RAM free: %lu | pre-allocating %d chunks, total size: %lu", info.freeram, chunk_count, chunk_count * (long)CHUNK_SIZE);
    chunks = (void **) malloc(chunk_count * sizeof(void*));
    assert_that(chunks != NULL, "map failed");
    
    alloc_chunks(chunks, chunk_count);
    malloc_bench_harness(test, n, size);
    free_chunks(chunks, chunk_count);

    return 0;
}
