#include "cr_malloc.h"
#include "pkru.h"
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h> /* for syscall(__NR_gettid) */
#include <err.h>

// Global memory pool.
static mspace global = NULL;
// Thread local reference to the memory pool that the thread should use (it can point to global).
//static __thread mspace local = NULL;
// Thread local variable that acts as it's TID.
// static __thread pid_t current_tid = 0;
// Array for storing mspaces for each sandbox.
static mspace mspace_table[MAX_MSPACE] = {0};
// Number of used mspaces.
static int mspace_count = 0;

// There are 16 workers, so we only need to store the TID of 16 threads
// A TID value that does not correspond to any worker TID is therefore a managed thread
// Managed code will use the global mspace, whereas native code will use individual mspaces
// NOTE (optimization): we use the fs register instead of TID to avoid executing a system call
// FIXME: What happens when a worker thread calls clone inside native?
//        We probably want the new thread to execute in the same memory domain and to
//        allocate memory in the same mspace as the parent!
static unsigned long worker_threads[16] = {0};
static unsigned long wrapper_threads[16] = {0};

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

void set_mspace(unsigned int pkey, mspace m) {
    mspace_table[pkey] = m;
}

mspace get_mspace(unsigned int pkey) {
    return mspace_table[pkey];
}

int get_mspace_count() {
    return mspace_count;
}

void init_global_mspace() {
    mspace newmspace = create_mspace(0, 0);
    mspace uninitialized = NULL;
    if (!__atomic_compare_exchange(&global, &uninitialized, &newmspace, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) {
        destroy_mspace(newmspace);
    } else {
        mspace_track_large_chunks(global, 1);
    }
}

mspace find_mspace() {
    mspace mem;
    int mspace_id;
    unsigned int tid;

    mspace_id = -1;
    tid = syscall(__NR_gettid);
    for (int i = 0; i < 16; i++) {
        if (tid == worker_threads[i] || tid == wrapper_threads[i]) {
            mspace_id = i;
            break;
        }
    }

    mem = mspace_id == -1 ? global : mspace_table[mspace_id];
    if (mem)
        return mem;

    if (mspace_id == -1) {
        init_global_mspace();
        return global;
    } else {
        mspace_table[mspace_id] = create_mspace(0, 0);
        return mspace_table[mspace_id];
    }
}

void* malloc(size_t bytes) {
    unsigned int privileged_domain;
    unsigned int unprivileged_domain;
    
    unprivileged_domain = __rdpkru();
    privileged_domain = unprivileged_domain & 0x55555554;

    __wrpkrumem(privileged_domain);
    void* ret = mspace_malloc(find_mspace(), bytes);
    __wrpkrumem(unprivileged_domain);

    return ret;
}

void free(void* mem) {
    return mspace_free(find_mspace(), mem);
}

void* calloc(size_t num, size_t size){
    void* ret = mspace_calloc(find_mspace(), num, size);
    return ret;
}

void* realloc(void* ptr, size_t size){
    void* ret = mspace_realloc(find_mspace(), ptr, size);
    return ret;
}
