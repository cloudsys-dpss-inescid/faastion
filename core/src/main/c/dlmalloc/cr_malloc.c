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

#define switch_privileged \
unsigned int pku = __rdpkru();                                      \
unsigned int privileged_pku = pku & DOMAIN_TO_PKRU(LOADER_DOMAIN);  \
__wrpkrumem(privileged_pku)

#define switch_unprivileged __wrpkrumem(pku)

typedef struct {
    int locked_tid;
    int value;
    int n;
} futex_semaphore;

static mspace mspace_table[MAX_MSPACE] = {0};

#ifdef MUTEX_LOCKING
static pthread_mutex_t mutex_table[MAX_MSPACE] = {0};
static pthread_mutex_t *mutex_ptr_table[MAX_MSPACE] = {0};
#else
static futex_semaphore *sem_table[MAX_MSPACE] = {0};
#endif

static int use_cached_tid = 0;
pid_t (*get_cached_tid)(void) = NULL;
void (*set_cached_tid)(pid_t) = NULL;

char *__msids = NULL;

#include "util.h"

pid_t get_current_tid() {
    pid_t tid;
    if (use_cached_tid) {
        tid = get_cached_tid();
        if (tid == 0) {
            tid = syscall(__NR_gettid);
            set_cached_tid(tid);
        }
    } else {
        tid = syscall(__NR_gettid);
    }
    return tid;
}

void dlmalloc_init(char *msids, pid_t (*__get_cached_tid)(void), void (*__set_cached_tid)(pid_t)) {
    __msids = msids;
    use_cached_tid = 1;
    get_cached_tid = __get_cached_tid;
    set_cached_tid = __set_cached_tid;
    pkey_mprotect(get_mstate(), malloc_state_sz, PROT_READ | PROT_WRITE, LOADER_DOMAIN);
}

void ensure_msid(unsigned int tid, unsigned int mspace_id) {
    __msids[tid] = mspace_id;
}

void worker_mspace_init(unsigned int pkey, mspace m, void *lock, void *gm, char *msids, pid_t (*__get_cached_tid)(void), void (*__set_cached_tid)(pid_t)) {
    init_mparams();
    set_mstate(gm);
    use_cached_tid = 1;
    get_cached_tid = __get_cached_tid;
    set_cached_tid = __set_cached_tid;
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
    mspace mem = mspace_table[mspace_id];
    if (!mem)
        mem = init_mspace(mspace_id);
    return mem;
}

static mspace find_mspace(int tid) {
    return get_mspace(get_mspace_id(tid));
}

void* malloc(size_t bytes) {
    switch_privileged;
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] malloc, mspace_id: %d\n", tid, mspace_id);
    acquire_lock();
    void* ret = mspace_malloc(get_mspace(mspace_id), bytes);
    release_lock();
    debug_dump("malloc: %ld\n", (unsigned long)ret);
    switch_unprivileged;
    return ret;
}

void free(void* mem) {
    switch_privileged;
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] free: %ld, mspace_id: %d\n", tid, (unsigned long)mem, mspace_id);
    acquire_lock();
    mspace_free(get_mspace(mspace_id), mem);
    release_lock();
    debug_dump("free: success\n");
    switch_unprivileged;
}

void* calloc(size_t num, size_t size) {
    switch_privileged;
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] calloc, mspace_id: %d\n", tid, mspace_id);
    acquire_lock();
    void* ret = mspace_calloc(get_mspace(mspace_id), num, size);
    release_lock();
    debug_dump("calloc: %ld\n", (unsigned long)ret);
    switch_unprivileged;
    return ret;
}

void* realloc(void* ptr, size_t size) {
    switch_privileged;
    int tid = get_current_tid();
    int mspace_id = get_mspace_id(tid);
    debug_dump("[%d] realloc, mspace_id: %d\n", tid, mspace_id);
    acquire_lock();
    void* ret = mspace_realloc(get_mspace(mspace_id), ptr, size);
    release_lock();
    debug_dump("realloc: %ld\n", (unsigned long)ret);
    switch_unprivileged;
    return ret;
}

size_t malloc_usable_size(const void* mem) {
    switch_privileged;
    debug_dump("malloc_usable_size\n");
    size_t ret = mspace_usable_size(mem);
    switch_unprivileged;
    return ret;
}

struct mallinfo mallinfo() {
    switch_privileged;
    int tid = get_current_tid();
    debug_dump("[%d] mallinfo\n", tid);
    struct mallinfo ret = mspace_mallinfo(find_mspace(tid));
    switch_unprivileged;
    return ret;
}

int mallopt(int param_number, int value) {
    switch_privileged;
    debug_dump("mallopt\n");
    int ret = mspace_mallopt(param_number, value);
    switch_unprivileged;
    return ret;
}

void* memalign(size_t alignment, size_t bytes) {
    switch_privileged;
    int tid = get_current_tid();
    debug_dump("[%d] memalign\n", tid);
    void *ret = mspace_memalign(find_mspace(tid), alignment, bytes);
    switch_unprivileged;
    return ret;
}

int posix_memalign(void **memptr, size_t alignment, size_t size) {
    switch_privileged;
    int ret = ENOMEM;
    int tid = get_current_tid();
    debug_dump("[%d] posix_memalign\n", tid);
    void *mem = mspace_memalign(find_mspace(tid), alignment, size);
    if (mem) {
        *memptr = mem;        
        ret = 0;
    }
    switch_unprivileged;
    return ret;
}

void* valloc(size_t size) {
    switch_privileged;
    int tid = get_current_tid();
    debug_dump("[%d] valloc\n", tid);
    void *ret = mspace_memalign(find_mspace(tid), getpagesize(), size);
    switch_unprivileged;
    return ret;
}

void* pvalloc(size_t size) {
    switch_privileged;
    int tid = get_current_tid();
    int pagesize = getpagesize();
    debug_dump("[%d] pvalloc\n", tid);
    void *ret = mspace_memalign(find_mspace(tid), pagesize, (size + pagesize - (size_t)1) & ~(pagesize - (size_t)1));
    switch_unprivileged;
    return ret;
}

void malloc_stats() {
    switch_privileged;
    int tid = get_current_tid();
    debug_dump("[%d] malloc_stats\n", tid);
    mspace_malloc_stats(find_mspace(tid));
    switch_unprivileged;
}

int malloc_trim(size_t pad) {
    switch_privileged;
    int tid = get_current_tid();
    debug_dump("[%d] malloc_trim\n", tid);
    int ret = mspace_trim(find_mspace(tid), pad);
    switch_unprivileged;
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