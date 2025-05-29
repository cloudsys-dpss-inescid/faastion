#include "pkru.h"
#include "cr_malloc.h"

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

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


// There are 16 workers, so we only need to store the TID of 16 threads
// A TID value that does not correspond to any worker TID is therefore a managed thread
// Managed code will use the global mspace, whereas native code will use individual mspaces
// NOTE (optimization): we use the fs register instead of TID to avoid executing a system call
// FIXME: What happens when a worker thread calls clone inside native?
//        We probably want the new thread to execute in the same memory domain and to
//        allocate memory in the same mspace as the parent!
static unsigned long worker_threads[DOMAINS] = {0};
static unsigned long wrapper_threads[DOMAINS] = {0};

#include "util.h"

void register_worker_thread(unsigned int pkey, unsigned int tid) {
    worker_threads[pkey] = tid;
}

void register_wrapper_thread(unsigned int pkey, unsigned int tid) {
    wrapper_threads[pkey] = tid;
    mspace_count++;
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

void set_mspace_lock(unsigned int pkey, void *lock) {
#ifdef MUTEX_LOCKING
    mutex_ptr_table[pkey] = (pthread_mutex_t *)lock;
#else
    sem_table[pkey] = (futex_semaphore *)lock;
#endif
}

void set_mspace(unsigned int pkey, mspace m) {
    mspace_table[pkey] = m;
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

int get_mspace_id(int tid) {
    int i;
    for (i = 0; i < DOMAINS; i++) {
        if (tid == worker_threads[i] || tid == wrapper_threads[i]) {
            break;
        }
    }
    return i;
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
