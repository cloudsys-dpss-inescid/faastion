#include "mmap.h"
#include "utils.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

test_mmap_t find_mmap_t(test_mmap_t test_functions[], int n, const char* test_name) 
{
    test_mmap_t test = { NULL, "unknown" };
    for (int i = 0; i < n; i++) {
        if (strcmp(test_functions[i].name, test_name) == 0) {
            test = test_functions[i];
        }
    }

    assert_that(test.function != NULL, "failed to locate test function with such name");
    return test;
}

void mmap_bench_harness(test_mmap_t func, int n, void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    double accum = 0;

    for (int i = 0; i < n; i++) {
        double t = func.function(addr, length, prot, flags, fd, offset); 
        printf("%.9lf\n", t);
        fflush(stdout);
        accum += t;
    }

    double avg = accum / n;
    UTIL_LOGF("avg: %.9lf", avg);
}

double mmap_init(void *addr, size_t length, int prot, int flags, int fd, off_t offset) 
{
    UTIL_CLOCK_INIT
    
    UTIL_CLOCK_START;
        void *mem = mmap(addr, length, prot, flags, fd, offset);
        assert_that(mem != MAP_FAILED, "mmap failed");
        for(size_t i = 0; i < length; i++) {
            ((char *)mem)[i] = 0;
        }
    UTIL_CLOCK_STOP;
    munmap(mem, length);

    return UTIL_CLOCK_DELTA;
}

double mmap_no_init(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    UTIL_CLOCK_INIT
    
    UTIL_CLOCK_START;
        void *mem = mmap(addr, length, prot, flags, fd, offset);
        assert_that(mem != MAP_FAILED, "mmap failed");
    UTIL_CLOCK_STOP;
    munmap(mem, length);

    return UTIL_CLOCK_DELTA;
}

double mmap_init_no_unmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    UTIL_CLOCK_INIT
    
    UTIL_CLOCK_START;
        void *mem = mmap(addr, length, prot, flags, fd, offset);
        assert_that(mem != MAP_FAILED, "mmap failed");
        for(size_t i = 0; i < length; i++) {
            ((char *)mem)[i] = 0;
        }
    UTIL_CLOCK_STOP;

    return UTIL_CLOCK_DELTA;
}

double mmap_no_init_no_unmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    UTIL_CLOCK_INIT
    
    UTIL_CLOCK_START;
        void *mem = mmap(addr, length, prot, flags, fd, offset);
        assert_that(mem != MAP_FAILED, "mmap failed");
    UTIL_CLOCK_STOP;

    return UTIL_CLOCK_DELTA;
}
