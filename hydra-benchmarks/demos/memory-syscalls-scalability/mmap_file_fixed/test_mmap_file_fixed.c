#include <sys/mman.h>

#include "../utils/utils.h"
#include "../utils/mmap.h"

int main(int argc, char** argv) 
{
    assert_that(argc >= 3, "expected test_case_name and iter_count");

    test_mmap_t test_functions[] = {
        { mmap_no_init, "mmap_no_init" },
        { mmap_init, "mmap_init" },
        { mmap_no_init_no_unmap, "mmap_no_init_no_unmap" },
        { mmap_init_no_unmap, "mmap_init_no_unmap" },
    };

    test_mmap_t test = find_mmap_t(test_functions, sizeof(test_functions) / sizeof(test_functions[0]), argv[1]);

    int n = atoi(argv[2]);
    int size = ALLOC_SIZE;
    if (argc > 3) {
        size = atoi(argv[3]);
    }

    UTIL_LOGF("config: %s , n=%d , size=%d", test.name, n, size);
    
    int fd = create_temp_file("/tmp/bench_test_mmap_file.bin", size);
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, fd, 0);
    mmap_bench_harness(test, n, addr, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, fd, 0);
    munmap(addr, size);

    return 0;
}
