#ifndef __UTIL_H__
#define __UTIL_H__

extern void print(char *fmt, ...);
extern futex_semaphore *new_semaphore(int value);
extern void acquire(futex_semaphore *sem);
extern void release(futex_semaphore *sem);


#ifdef DEBUG_MODE
#define debug_dump(fmt, ...) (print(fmt __VA_OPT__(,) __VA_ARGS__))
#else
void debug_dump(__attribute__((unused)) char *fmt, ...) {}
#endif

#ifdef MUTEX_LOCKING
#define acquire_lock() \
    pthread_mutex_t *mutex = mutex_ptr_table[mspace_id];                            \
    if (mutex)                                                                      \
        pthread_mutex_lock(mutex);
#else
#define acquire_lock() \
    futex_semaphore *sem = sem_table[mspace_id];                                    \
    if (sem && sem->locked_tid != tid) {                                            \
        acquire(sem);                                                               \
        sem->locked_tid = tid;                                                      \
    }
#endif

#ifdef MUTEX_LOCKING
#define release_lock() \
    if (mutex)                                                                      \
        pthread_mutex_unlock(mutex);
#else
#define release_lock() \
    if (sem) {                                                                      \
        sem->locked_tid = 0;                                                        \
        release(sem);                                                               \
    }
#endif

int get_current_tid() {
#ifdef MSPACE_CACHING
    if (!current_tid)
        current_tid = syscall(__NR_gettid);
#else
    int current_tid = syscall(__NR_gettid);
#endif
    return current_tid;
}

#endif