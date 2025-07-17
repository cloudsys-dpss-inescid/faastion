#define _GNU_SOURCE

#include "pkru.h"
#include "cr_malloc.h"

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/syscall.h>


// #define MUTEX_LOCKING 1

#ifdef MSPACE_CACHING
static __thread mspace local = NULL;
static __thread pid_t current_tid = 0;
#endif

typedef struct {
    int locked_tid;
    int value;
} futex_semaphore;

static mspace mspace_table[MAX_MSPACE] = {0};

#ifdef MUTEX_LOCKING
static pthread_mutex_t mutex_table[MAX_MSPACE] = {0};
static pthread_mutex_t *mutex_ptr_table[MAX_MSPACE] = {0};
#else
static futex_semaphore *sem_table[MAX_MSPACE] = {0};
#endif

static int mspace_count = 0;
char *__msids = NULL;

#include "util.h"

void dlmalloc_init(char *msids) {
    __msids = msids;
    pkey_mprotect(get_mstate(), malloc_state_sz, PROT_READ | PROT_WRITE, LOADER_DOMAIN);
}

void ensure_msid(unsigned int tid, unsigned int mspace_id) {
    __msids[tid] = mspace_id;
}

void worker_mspace_init(unsigned int pkey, mspace m, void *lock, void *gm, char *msids) {
    init_mparams();
    set_mstate(gm);
    mspace_table[pkey] = m;
#ifdef MUTEX_LOCKING
    mutex_ptr_table[pkey] = (pthread_mutex_t *)lock;
#else
    sem_table[pkey] = (futex_semaphore *)lock;
#endif
    __msids = msids;
    __msids[syscall(__NR_gettid)] = pkey;
}

mspace get_mspace_mapping() {
    return mspace_table;
}

void *get_mspace_lock(unsigned int pkey) {
#ifdef MUTEX_LOCKING
    return (void *)&mutex_table[pkey];
#else
    return (void *)sem_table[pkey];
#endif
}

int get_mspace_count() {
    return mspace_count;
}

#ifdef MUTEX_LOCKING
void init_mspace_mutex(int mspace_id) {
    static pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutex_table[mspace_id], &attr);
    mutex_ptr_table[mspace_id] = &mutex_table[mspace_id];
}
#else
void init_mspace_sem(int mspace_id) {
    sem_table[mspace_id] = new_semaphore(1);
}
#endif

mspace init_mspace(int mspace_id) {
    mspace newmspace = create_mspace(0, 0);
    mspace_table[mspace_id] = newmspace;
    mspace_track_large_chunks(mspace_table[mspace_id], 1);
#ifdef MUTEX_LOCKING
    init_mspace_mutex(mspace_id);
#else
    init_mspace_sem(mspace_id);
#endif
    return newmspace;
}

static int get_mspace_id(int tid) {
    int pkru;
    int mspace_id;

    if (!__msids)
        return 0;
    
    pkru = __rdpkru();
    __wrpkru(DEFAULT_DOMAIN);
    mspace_id = (int)__msids[tid];
    __wrpkrumem(pkru);

    return mspace_id;
}

mspace get_mspace(unsigned int mspace_id) {
#ifdef MSPACE_CACHING
    if (local)
        return local;
#endif

    mspace mem = mspace_table[mspace_id];
    if (mem)
        return mem;
    else
        mem = init_mspace(mspace_id);

#ifdef MSPACE_CACHING
    local = mem;
#endif

    return mem;
}

static mspace find_mspace(int tid) {
    return get_mspace(get_mspace_id(tid));
}

void* malloc(size_t bytes) {
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] malloc, mspace_id: %d\n", tid, mspace_id);
    acquire_lock();
    void* ret = mspace_malloc(get_mspace(mspace_id), bytes);
    release_lock();
    debug_dump("malloc: %ld\n", (unsigned long)ret);
    return ret;
}

void free(void* mem) {
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] free: %ld, mspace_id: %d\n", tid, (unsigned long)mem, mspace_id);
    acquire_lock();
    mspace_free(get_mspace(mspace_id), mem);
    release_lock();
    debug_dump("free: success\n");
}

void* calloc(size_t num, size_t size) {
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] calloc, mspace_id: %d\n", tid, mspace_id);
    acquire_lock();
    void* ret = mspace_calloc(get_mspace(mspace_id), num, size);
    release_lock();
    debug_dump("calloc: %ld\n", (unsigned long)ret);
    return ret;
}

void* realloc(void* ptr, size_t size) {
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] realloc, mspace_id: %d\n", tid, mspace_id);
    acquire_lock();
    void* ret = mspace_realloc(get_mspace(mspace_id), ptr, size);
    release_lock();
    debug_dump("realloc: %ld\n", (unsigned long)ret);
    return ret;
}

size_t malloc_usable_size(const void* mem) {
    size_t ret = dlmalloc_usable_size(mem);
    return ret;
}

struct mallinfo mallinfo() {
    int tid = get_current_tid();
    struct mallinfo ret = mspace_mallinfo(find_mspace(tid));
    return ret;
}

int mallopt(int param_number, int value) {
    int ret = dlmallopt(param_number, value);
    return ret;
}

void* memalign(size_t alignment, size_t bytes) {
    int tid = get_current_tid();
    void *ret = mspace_memalign(find_mspace(tid), alignment, bytes);
    return ret;
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    int ret = dlposix_memalign(memptr, alignment, size);
    return ret;
}

void* valloc(size_t size) {
    void *ret = dlvalloc(size);
    return ret;
}

void* pvalloc(size_t size) {
    void *ret = dlpvalloc(size);
    return ret;
}

void malloc_stats() {
    int tid = get_current_tid();
    mspace_malloc_stats(find_mspace(tid));
}

int malloc_trim(size_t pad) {
    int tid = get_current_tid();
    int ret = mspace_trim(find_mspace(tid), pad);
    return ret;
}

int malloc_info() {
    debug_dump("error: malloc_info is not supported\n");
    return 0;
}

void *reallocarray (void *__ptr, size_t __nmemb, size_t __size) {
    debug_dump("error: reallocarray is not supported\n");
    return NULL;
}