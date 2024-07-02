#include "../utils/utils.h"
#include "../utils/malloc.h"

double free_init(size_t length)
{
    void *mem = malloc(length);
    assert_that(mem != NULL, "malloc failed");
    for(size_t i = 0; i < length; i++) {
        ((char *)mem)[i] = 0;
    }

    UTIL_CLOCK_INIT
    UTIL_CLOCK_START;
        free(mem);
    UTIL_CLOCK_STOP;

    return UTIL_CLOCK_DELTA;
}

double free_no_init(size_t length)
{
    void *mem = malloc(length);
    assert_that(mem != NULL, "malloc failed");

    UTIL_CLOCK_INIT
    UTIL_CLOCK_START;
        free(mem);
    UTIL_CLOCK_STOP;

    return nanos(start, stop);
}

int main(int argc, char** argv) 
{
    assert_that(argc >= 3, "expected test_case_name and iter_count");

    test_malloc_t test_functions[] = {
        { free_no_init, "free_no_init" },
        { free_init, "free_init" },
    };

    test_malloc_t test = find_malloc_t(test_functions, sizeof(test_functions) / sizeof(test_functions[0]), argv[1]);

    int n = atoi(argv[2]);
    int size = ALLOC_SIZE;  
    if (argc > 3) {
        size = atoi(argv[3]);
    }

    UTIL_LOGF("config: %s , n=%d , size=%d", test.name, n, size);
    malloc_bench_harness(test, n, size);

    return 0;
}
