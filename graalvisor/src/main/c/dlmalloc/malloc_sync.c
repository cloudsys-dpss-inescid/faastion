#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <unistd.h>
#include <stdatomic.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <sys/syscall.h>

#include <linux/futex.h>

typedef struct {
    int locked_tid;
    int value;
} futex_semaphore;

static int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

futex_semaphore *new_semaphore(int value) {
    futex_semaphore *sem = (futex_semaphore *)mmap(NULL, sizeof(futex_semaphore),
                PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
    sem->value = value;
    sem->locked_tid = 0;
    return sem;
}

void destroy_semaphore(futex_semaphore *sem) {
    munmap(sem, sizeof(futex_semaphore));
}

// TODO: create spin-locks
void acquire(futex_semaphore *sem) {
    int semaphore_value = 0;
    for (;;) {
        while ((semaphore_value = atomic_load(&sem->value)) == 0) ;
        if (atomic_compare_exchange_strong(&sem->value, &semaphore_value, semaphore_value-1)) {
            return;
        }
    }

    // do {
    //     int semaphore_value = atomic_load(&sem->value);
    //     if (semaphore_value == 0) {
    //         continue;
    //     }
    //     if (atomic_compare_exchange_strong(&sem->value, &semaphore_value, semaphore_value-1)) {
    //         return;
    //     }
    // } while (futex(&sem->value, FUTEX_WAIT, 0, NULL, NULL, 0) == 0 || errno == EAGAIN);
}

// TODO: create spin-locks
static inline void futex_release(futex_semaphore *sem, int waiters) {
    atomic_fetch_add(&sem->value, 1);
    // futex(&sem->value, FUTEX_WAKE, waiters, NULL, NULL, 0);
}

void release(futex_semaphore *sem) {
    futex_release(sem, 1);
}

void release_all(futex_semaphore *sem) {
    futex_release(sem, INT_MAX);
}