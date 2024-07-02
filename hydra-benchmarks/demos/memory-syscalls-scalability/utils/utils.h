#ifndef __UTILS__UTILS_H
#define __UTILS__UTILS_H

#include <stddef.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BILLION     1000000000L
#define ALLOC_SIZE  (1024 * 1024)

#define UNUSED(arg) ((void)arg);

#define UTIL_CLOCK_INIT  struct timespec start, stop;
#define UTIL_CLOCK_START assert_that(clock_gettime(CLOCK_MONOTONIC, &start) != -1, "clock_gettime start")
#define UTIL_CLOCK_STOP  assert_that(clock_gettime(CLOCK_MONOTONIC, &stop) != -1, "clock_gettime stop")
#define UTIL_CLOCK_DELTA nanos(start, stop)

#define UTIL_LOG(fmt) \
    do { \
        fprintf(stderr, "[%s:%d]: ", __FILE__, __LINE__);   \
        fprintf(stderr, fmt"\n");                           \
    } while (0)

#define UTIL_LOGF(fmt, ...) \
    do { \
        fprintf(stderr, "[%s:%d]: ", __FILE__, __LINE__);   \
        fprintf(stderr, fmt"\n", __VA_ARGS__);              \
    } while (0)

int assert_that(int v, const char* msg);
double nanos(struct timespec start, struct timespec stop);
int create_temp_file(const char *path, size_t length);

#endif
