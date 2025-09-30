#define _GNU_SOURCE
#include <pthread.h>
#include "cr_malloc.h"
#include "../../cr_logger.h"
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h> /* for syscall(__NR_gettid) */
#include <err.h>

// If defined, enables debug prints and extra sanitization checks.
//#define HYDRALLOC_DEBUG

#ifdef HYDRALLOC_DEBUG
    #define dbg(format, args...) do { cr_printf(STDOUT_FILENO, format, ## args); } while(0)
#else
    #define dbg(format, args...) do { } while(0)
#endif

// Thread local reference to the memory pool that the thread should use (it can point to global).
static __thread mspace local = NULL;
// Thread local variable that acts as it's TID.
static __thread pid_t current_tid = 0;
// Array for storing mspaces for each sandbox.
static mspace mspace_table[MAX_MSPACE] = {0};
// Number of used mspaces.
static volatile int mspace_count = 0;
// Array for storing mutex for each sandbox.
static pthread_mutex_t mutex_table[MAX_MSPACE] = {0};
// Array for storing mutex attribute for each sandbox.
static pthread_mutexattr_t attr_table[MAX_MSPACE] = {0};
// Mutex for exlusive access to initialization of an mspace.
pthread_mutex_t malloc_mutex = PTHREAD_MUTEX_INITIALIZER;

// If the memory allocator becomes the bottleneck it might be worth to re-evaluate thread_locals:
// https://stackoverflow.com/questions/9909980/how-fast-is-thread-local-variable-access-on-linux

void* get_mem_allocator_addr() {
    // we skip global mspace because it can't be checkpoint/restored
    return &mspace_table[1];
}

int get_mem_allocator_len() {
    return (mspace_count - 1) * sizeof(mspace);
}

void print_mspace(mstate mapping) {
    cr_printf(STDOUT_FILENO, "seg.base @ %16p, dv @ %16p with size %zu\n", mapping->seg.base, mapping->dv, mapping->dvsize);
}

void print_allocator() {
    int mspace_id = 1;
    while (mspace_table[mspace_id]) {
        print_mspace(mspace_table[mspace_id++]);
    }
}

void init_mspace(int mspace_id) {
    pthread_mutex_lock(&malloc_mutex);
    dbg("[HYDRALLOC] inside init_mspace from tid=%d\n", current_tid);
    if (!mspace_table[mspace_id]) {
        mspace newmspace = create_mspace(0, 0);
        mspace_table[mspace_id] = newmspace;
        mspace_track_large_chunks(mspace_table[mspace_id], 1);
        pthread_mutexattr_init(&attr_table[mspace_id]);
        pthread_mutexattr_settype(&attr_table[mspace_id], PTHREAD_MUTEX_RECURSIVE);
        if (pthread_mutex_init(&mutex_table[mspace_id], &attr_table[mspace_id]) != 0) {
            cr_printf(STDOUT_FILENO, "Mutex initialization failed\n");
        }
        dbg("[HYDRALLOC] created mspace %d from tid %d -> %p\n", mspace_count, current_tid, newmspace);
        mspace_count++;
    }
    pthread_mutex_unlock(&malloc_mutex);
}

mspace find_mspace() {
    if (local) {
        return local;
    }

    if (!current_tid) {
        current_tid = syscall(__NR_gettid);
    }
    // index 0 is used for global mspace
    int mspace_id = (current_tid / 1000);

    if (!mspace_table[mspace_id]) {
        init_mspace(mspace_id);
    }

    local = mspace_table[mspace_id];
    return local;
}

mspace lock_mspace(int mspace_id) {
    mspace ms = find_mspace();
    pthread_mutex_lock(&mutex_table[mspace_id]);
    dbg("[HYDRALLOC] locked mspace %d from tid %d\n", mspace_id, current_tid);
    return ms;
}

void unlock_mspace(int mspace_id) {
    dbg("[HYDRALLOC] unlocking mspace %d from tid %d\n", mspace_id, current_tid);
    pthread_mutex_unlock(&mutex_table[mspace_id]);
}

void* malloc(size_t bytes) {
    find_mspace();
    dbg("[HYDRALLOC] inside malloc from tid=%d\n", current_tid);
    int mspace_id = (current_tid / 1000);
    mspace ms = lock_mspace(mspace_id);
    void* ret = mspace_malloc(ms, bytes);
    dbg("[HYDRALLOC] malloc(mspace = %p, bytes = %lu) -> %p\n", ms, bytes, ret);
    unlock_mspace(mspace_id);
    return ret;
}

void free(void* mem) {
    find_mspace();
    dbg("[HYDRALLOC] inside free of %p from tid=%d with local=%p and gettid=%d\n", mem, current_tid, local, gettid());
    int mspace_id = (current_tid / 1000);
    mspace ms = lock_mspace(mspace_id);
    // compiled with FOOTERS=1 this mspace_free() will be replaced by a free()
    // so global mspace can call free of stuff on the application's mspace
    mspace_free(ms, mem);
    dbg("[HYDRALLOC] free(mspace = %p, mem = %p)\n", ms, mem);
    unlock_mspace(mspace_id);
    return;
}

void* calloc(size_t num, size_t size) {
    find_mspace();
    dbg("[HYDRALLOC] inside calloc from tid=%d\n", current_tid);
    int mspace_id = (current_tid / 1000);
    mspace ms = lock_mspace(mspace_id);
    void* ret = mspace_calloc(ms, num, size);
    dbg("[HYDRALLOC] calloc(mspace = %p, num = %lu, size = %lu) -> %p\n", ms, num, size, ret);
    unlock_mspace(mspace_id);
    return ret;
}

void* realloc(void* ptr, size_t size) {
    find_mspace();
    dbg("[HYDRALLOC] inside realloc from tid=%d\n", current_tid);
    int mspace_id = (current_tid / 1000);
    mspace ms = lock_mspace(mspace_id);
    void* ret = mspace_realloc(ms, ptr, size);
    dbg("[HYDRALLOC] realloc(mspace = %p, ptr = %p, size = %lu) -> %p\n", ms, ptr, size, ret);
    unlock_mspace(mspace_id);
    return ret;
}

size_t malloc_usable_size(const void* mem) {
    return dlmalloc_usable_size(mem);
}

struct mallinfo mallinfo() {
    return mspace_mallinfo(find_mspace());
}

int mallopt(int param_number, int value) {
    return dlmallopt(param_number, value);
}

void* memalign(size_t alignment, size_t bytes) {
    return mspace_memalign(find_mspace(), alignment, bytes);
}

void* valloc(size_t size) {
    return dlvalloc(size);
}

void* pvalloc(size_t size) {
    return dlpvalloc(size);
}

void malloc_stats() {
    mspace_malloc_stats(find_mspace());
}

int malloc_info() {
    err(1, "error: malloc_info is not supported)");
    return 0;
}

int malloc_trim(size_t pad) {
    return mspace_trim(find_mspace(), pad);
}

void *reallocarray (void *__ptr, size_t __nmemb, size_t __size) {
    err(1, "error: reallocarray is not supported)");
    return NULL;
}
