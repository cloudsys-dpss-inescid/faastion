#ifndef __UTILS__MMAP_H
#define __UTILS__MMAP_H

#include <stdlib.h>
#include <sys/types.h>

typedef struct {
    double (*function)(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
    char *name;
} test_mmap_t;

test_mmap_t find_mmap_t(test_mmap_t functions[], int n, const char* name);

void mmap_bench_harness(test_mmap_t func, int n, void *addr, size_t length, int prot, int flags, int fd, off_t offset);
double mmap_init(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
double mmap_no_init(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
double mmap_init_no_unmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
double mmap_no_init_no_unmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);

#endif
