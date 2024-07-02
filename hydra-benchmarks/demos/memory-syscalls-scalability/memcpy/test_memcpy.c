#include "../utils/utils.h"
#include "../utils/malloc.h"
#include <string.h>

double memcpy_test(size_t length)
{
    void *src = malloc(length);
    assert_that(src != NULL, "malloc failed");
    void *dst = malloc(length);
    assert_that(dst != NULL, "malloc failed");

    UTIL_CLOCK_INIT
    UTIL_CLOCK_START;
        memcpy(dst, src, length);
    UTIL_CLOCK_STOP;

    free(src);
    free(dst);
    return UTIL_CLOCK_DELTA;
}

int main(int argc, char** argv) 
{
    assert_that(argc >= 3, "expected test_case_name and iter_count");

    test_malloc_t test_functions[] = {
        { memcpy_test, "memcpy_test" },
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
