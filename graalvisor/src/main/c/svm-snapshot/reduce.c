#include "cr.h"
#include "syscalls.h"

#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/time.h>

// If defined, enables debug prints.
// #define MINIMIZE_DEBUG

#ifdef MINIMIZE_DEBUG
    #define mdbg(format, args...) do { cr_printf(STDOUT_FILENO, format, ## args); } while(0)
#else
    #define mdbg(format, args...) do { } while(0)
#endif

// Arbitrary value to set size of static array where we store maps_t.
#define MAX_MMAP_COUNT 1000
// Arbitrary value to set initial size of dynamic array where we store syscalls.
#define INITIAL_LINES 100
// Arbitrary value to set size of static array where we store the syscall in binary.
#define MAX_DATA 500

typedef struct {
    // Binary data read from file that corresponds to a syscall to be restored.
    char data[MAX_DATA];
    // Number of the syscall.
    int syscall_nr;
    // Length of the data part, excludes syscall number.
    int data_len;
    // Tainted = 1 means this syscall is needed for a correct restore.
    int tainted;
} syscall_t;

typedef struct {
    // Saves state of a FD, closed / opened / opened AND required.
    int used;
    // Line from syscall array that opened this FD.
    int from_line;
} fd_t;

typedef struct {
    // Base of memory map.
    uintptr_t base;
    // Length of memory map.
    size_t length;
    // Line from syscall array that created this MMAP.
    int from_line;
} maps_t;

typedef struct {
    // Pointer to array.
    syscall_t* array;
    // Number of in-use elements of dynamic array.
    size_t used;
    // Max element capacity of dynamic array.
    size_t capacity;
} dynamic_array_t;

void init_dynamic(dynamic_array_t* dyn, size_t initial_size) {
    dyn->array = calloc(initial_size * sizeof(syscall_t), 1);
    dyn->used = 0;
    dyn->capacity = initial_size;
}

void insert_dynamic(dynamic_array_t* dyn, syscall_t* syscall) {
    if (dyn->used == dyn->capacity) {
        dyn->capacity *= 2;
        syscall_t* new_arr = realloc(dyn->array, dyn->capacity * sizeof(syscall_t));
        if (!new_arr) {
            err("insert_dynamic: buy more RAM, couldn't allocate capacity = %d", dyn->capacity);
            return;
        }
        dyn->array = new_arr;
    }
    dyn->array[dyn->used++] = *syscall;
}

void free_dynamic(dynamic_array_t* dyn) {
    free(dyn->array);
    dyn->array = NULL;
    dyn->used = 0;
    dyn->capacity = 0;
}

int get_syscall_size(int tag) {
    switch (tag)
        {
        case __NR_mmap:
            return sizeof(mmap_t);

        case __NR_munmap:
            return sizeof(munmap_t);

        case __NR_mprotect:
            return sizeof(mprotect_t);

        case __NR_dup:
            return sizeof(dup_t);

        case __NR_dup2:
            return sizeof(dup2_t);

        case __NR_open:
            return sizeof(open_t);

        case __NR_openat:
            return sizeof(openat_t);

        case __NR_close:
            return sizeof(close_t);

        default:
            err("error: unknown tag during get_syscall_size: %d", tag);
            return -1;
        }
}

void copy_line(int dst, int src, int tag, int size) {
    char buf[size];
    ssize_t bytes_read;
    ssize_t bytes_written;

    bytes_written = write(dst, &tag, sizeof(tag));
    if (bytes_written != sizeof(tag)) {
        err("copy_line: couldn't write tag.\n");
    }
    bytes_read = read(src, buf, size);
    if (bytes_read != size) {
        err("copy_line: couldn't read contents.\n");
    }
    bytes_written = write(dst, buf, size);
    if (bytes_written != size) {
        err("copy_line: couldn't write contents.\n");
    }
}

void organize_syscall(syscall_t* syscall, int src_fd, int tag, int size) {
    char buf[500] = {0};
    ssize_t bytes_read;

    syscall->syscall_nr = tag;
    memcpy(syscall->data, &tag, sizeof(int));
    bytes_read = read(src_fd, buf, size);
    if (bytes_read != size) {
        err("organize_syscall: couldn't read from src_fd.\n");
    }
    if (size > sizeof(buf)) {
        err("organize_syscall: data size is larger than buf size (500 bytes).\n");
        return;
    }
    memcpy(syscall->data + sizeof(int), buf, size);
    syscall->data_len = size;
    return;
}

void parse_syscalls(dynamic_array_t* lines, int meta_snap_fd) {
    int syscall_size = 0;
    int tag;
    size_t n;
    int line_counter = 0;

    while(1) {
        n = read(meta_snap_fd, &tag, sizeof(int));
        // check if finished reading sycalls
        if (tag < 0) {
            // "un-read" tag that was past syscalls block
            lseek(meta_snap_fd, -sizeof(int), SEEK_CUR);
            break;
        } else if (n != sizeof(int)) {
            err("error: failed to read tag");
        }
        mdbg("line %d is syscall nr %d\n", line_counter, tag);
        line_counter++;

        syscall_size = get_syscall_size(tag);
        syscall_t* syscall = malloc(sizeof(syscall_t));
        organize_syscall(syscall, meta_snap_fd, tag, syscall_size);
        insert_dynamic(lines, syscall);
    }
}

void filter_syscalls(dynamic_array_t* lines) {
    fd_t fd_tracker[RESERVED_FDS] = {0};
    maps_t mmap_tracker[MAX_MMAP_COUNT] = {0};
    int line_counter = 0;
    int mmap_counter = 0;
    syscall_t* line_array = lines->array;
    int pagesize = getpagesize();

    for (line_counter = 0; line_counter < lines->used; line_counter++) {
        if (line_array[line_counter].syscall_nr == __NR_openat) {
            openat_t* syscall = (openat_t *) (line_array[line_counter].data + sizeof(int));
            int fd = syscall->ret;
            fd_tracker[fd].used = 1;
            fd_tracker[fd].from_line = line_counter;
            line_array[line_counter].tainted = 1;
            continue;
        }

        if (line_array[line_counter].syscall_nr == __NR_close) {
            close_t* syscall = (close_t *) (line_array[line_counter].data + sizeof(int));
            int fd = syscall->fd;
            if (fd_tracker[fd].used == 0) {
                // if fd is not open remove close
                line_array[line_counter].tainted = 0;
            } else if ((fd_tracker[fd].used == 1)) {
                // if fd is open but not used, remove both open and close
                line_array[line_counter].tainted = 0;
                line_array[fd_tracker[fd].from_line].tainted = 0;
            } else {
                // otherwise open is needed and so is close
                line_array[line_counter].tainted = 1;
            }
            fd_tracker[fd].used = 0;
            fd_tracker[fd].from_line = -69;
            continue;
        }

        // dup and dup2 act like open/openat, it opens new FD
        if (line_array[line_counter].syscall_nr == __NR_dup) {
            dup_t* syscall = (dup_t *) (line_array[line_counter].data + sizeof(int));
            int fd = syscall->ret;
            fd_tracker[fd].used = 1;
            fd_tracker[fd].from_line = line_counter;
            line_array[line_counter].tainted = 1;
            continue;
        }

        if (line_array[line_counter].syscall_nr == __NR_dup2) {
            dup2_t* syscall = (dup2_t *) (line_array[line_counter].data + sizeof(int));
            // check ret instead of newfd because syscall might have failed
            int fd = syscall->ret;
            fd_tracker[fd].used = 1;
            fd_tracker[fd].from_line = line_counter;
            line_array[line_counter].tainted = 1;
            continue;
        }

        if (line_array[line_counter].syscall_nr == __NR_mmap) {
            mmap_t* syscall = (mmap_t *) (line_array[line_counter].data + sizeof(int));
            int fd = syscall->fd;

            if (mmap_counter >= MAX_MMAP_COUNT) {
                err("filter_syscalls: mmap buffer is full");
            }
            mmap_tracker[mmap_counter].base = (uintptr_t) syscall->ret;
            mmap_tracker[mmap_counter].length = syscall->length;
            mmap_tracker[mmap_counter].from_line = line_counter;
            mmap_counter++;

            // if fd is used by mmap, the open can't be removed
            // we need to check that mmap uses a fd and that we have access to it
            // NOTE: recvmsg can receive an FD as a message, we only save its mmap
            if (fd != -1 && fd_tracker[fd].used != 0) {
                fd_tracker[fd].used = 2;
                line_array[fd_tracker[fd].from_line].tainted = 1;
            }
            line_array[line_counter].tainted = 1;
            continue;
        }

        if (line_array[line_counter].syscall_nr == __NR_munmap) {
            mmap_t* syscall = (mmap_t *) (line_array[line_counter].data + sizeof(int));
            int mmap_index = 0;

            while (mmap_tracker[mmap_index].base) {
                if (mmap_tracker[mmap_index].base == (uintptr_t) syscall->addr) {
                    break;
                }
                mmap_index++;
            }

            // if no matching mmap call
            if (!mmap_tracker[mmap_index].base) {
                line_array[line_counter].tainted = 1;
                continue;
            }

            // we need to add padding because our seccomp tracer adds padding to intercepted mmap syscalls
            if (syscall->length % pagesize) {
                size_t padding = (pagesize - (syscall->length % pagesize));
                syscall->length += padding;
            }

            if (mmap_tracker[mmap_index].length == syscall->length) {
                line_array[line_counter].tainted = 0;
                line_array[mmap_tracker[mmap_index].from_line].tainted = 0;
            }
            continue;
        }
        // if none of the above cases match the current syscall, it is assumed necessary
        line_array[line_counter].tainted = 1;
    }
    mdbg("filter_syscalls number of lines=%d\n", line_counter);
}

void save_optimized_syscalls(dynamic_array_t* lines, int dst_fd, int src_fd) {
    int line_counter = 0;
    int optimized_counter = 0;
    ssize_t bytes_written = 0;
    syscall_t* line_array = lines->array;
    int not_copied = 0;

    // while (line_array[line_counter].data[0]) {
    while (line_counter < lines->used) {
        if (line_array[line_counter].tainted) {
            bytes_written = write(dst_fd, line_array[line_counter].data, line_array[line_counter].data_len+sizeof(int));
            mdbg("Line %d has bytes_written=%d, expected=%d\n", line_counter, bytes_written, line_array[line_counter].data_len+sizeof(int));
            if (bytes_written != line_array[line_counter].data_len+sizeof(int)) {
                err("save_optimized_syscalls: failed to save line");
            }
            optimized_counter++;
        } else {
            not_copied++;
            mdbg("Line %d not_copied %d\n", line_counter, not_copied);
        }
        line_counter++;
    }
    cr_printf(STDOUT_FILENO, "Removed %d out of %d original syscalls ", not_copied, line_counter);
}

void copy_remains(int optimized_fd, int meta_snap_fd) {
    char buf[4096];
    ssize_t bytes_read;
    ssize_t bytes_written;

    while ((bytes_read = read(meta_snap_fd, buf, sizeof(buf))) > 0) {
        bytes_written = write(optimized_fd, buf, bytes_read);
        if (bytes_read != bytes_written) {
            err("error: failed to read in copy_remains");
            return;
        }
    }

    if (bytes_read < 0) {
        err("error: failed to drain file");
    }
}

void minimize_syscalls(
    const char* meta_snap_path,
    const char* output_path) {

    int meta_snap_fd;
    int optimized_fd;
    dynamic_array_t lines;
    const char* temp_path = "temp.snap";

    struct timeval st, et;
    gettimeofday(&st, NULL);
    init_dynamic(&lines, INITIAL_LINES);

    // Open the metadata file, contains syscall arguments, memory ranges, etc.
    meta_snap_fd = open(meta_snap_path, O_RDWR);
    if (meta_snap_fd < 0) {
        err("error: failed to open meta snapshot file");
    } else {
        meta_snap_fd = move_to_reserved_fd(meta_snap_fd);
    }

    if (strcmp(output_path, meta_snap_path)) {
        optimized_fd = open(output_path, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    } else {
        optimized_fd = open(temp_path, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    }

    if (optimized_fd < 0) {
        err("error: failed to open minimized file");
    } else {
        optimized_fd = move_to_reserved_fd(optimized_fd);
    }

    parse_syscalls(&lines, meta_snap_fd);
    filter_syscalls(&lines);
    save_optimized_syscalls(&lines, optimized_fd, meta_snap_fd);

    copy_remains(optimized_fd, meta_snap_fd);

    close(optimized_fd);
    close(meta_snap_fd);

    if (!strcmp(output_path, meta_snap_path)) {
        rename(temp_path, output_path);
    }
    gettimeofday(&et, NULL);
    cr_printf(STDOUT_FILENO, "in %lu us\n", ((et.tv_sec - st.tv_sec) * 1000000) + (et.tv_usec - st.tv_usec));
}