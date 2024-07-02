#include "../utils/utils.h"
#include "../utils/malloc.h"
#include <stdlib.h>
#include <sys/sysinfo.h>

int main(int argc, char** argv) 
{
    assert_that(argc >= 3, "expected test_case_name and iter_count");

    test_malloc_t test_functions[] = {
        { malloc_no_init, "malloc_no_init" },
        { malloc_init, "malloc_init" },
        { malloc_no_init_no_free, "malloc_no_init_no_free" },
        { malloc_init_no_free, "malloc_init_no_free" },
    };

    test_malloc_t test = find_malloc_t(test_functions, sizeof(test_functions) / sizeof(test_functions[0]), argv[1]);

    int n = atoi(argv[2]);
    int size = ALLOC_SIZE;  
    if (argc > 3) {
        size = atoi(argv[3]);
    }

    struct sysinfo info;
    sysinfo(&info);

    int chunk_count = info.freeram / CHUNK_SIZE;
    UTIL_LOGF("RAM free: %lu | pre-allocating %d chunks, total size: %lu", info.freeram, chunk_count, chunk_count * (long)CHUNK_SIZE);
    void **chunks = (void **) malloc(chunk_count * sizeof(void*));
    assert_that(chunks != NULL, "map failed");
    
    alloc_chunks(chunks, chunk_count);
    make_fragments(chunks, chunk_count, n);

    UTIL_LOGF("config: %s , n=%d , size=%d", test.name, n, size);
    malloc_bench_harness(test, n, size);

    free_chunks(chunks, chunk_count);

    return 0;
}
