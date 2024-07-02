#include "utils.h"
#include <fcntl.h>
#include <unistd.h>

int assert_that(int v, const char* msg) {
    if (!v) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return v;
}

double nanos(struct timespec start, struct timespec stop)
{
    return (stop.tv_sec - start.tv_sec) + (double)(stop.tv_nsec - start.tv_nsec) / BILLION; 
}

int create_temp_file(const char *path, size_t length)
{
    int flags = O_CREAT | O_TRUNC | O_RDWR;
    mode_t mode = S_IRUSR | S_IWUSR;

    int fd = open(path, flags, mode);
    assert_that(fd != -1, "problem opening file");

    long page_size = sysconf(_SC_PAGE_SIZE);
    size_t buf_size = length >= ALLOC_SIZE ? ALLOC_SIZE : (size_t)page_size;

    void *buf;
    assert_that(posix_memalign(&buf, page_size, buf_size) == 0, "failed to allocate buffer");

    for (size_t i = 0; i < buf_size; i++) {
        ((char *)buf)[i] = (char)i;
    }

    size_t bytes_remaining = length;
    while (bytes_remaining > 0) {
        size_t bytes_to_write = bytes_remaining > buf_size ? buf_size : bytes_remaining;
        ssize_t bytes_written = write(fd, buf, bytes_to_write);
        assert_that(bytes_written != -1, "problem writing bytes to file");
        bytes_remaining -= bytes_written;
    }

    close(fd);
    free(buf);

    fd = open(path, O_RDWR, mode);
    assert_that(fd != -1, "problem opening file");

    return fd;
}