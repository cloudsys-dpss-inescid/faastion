#define _GNU_SOURCE

#include "cr.h"
#include "list_threads.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ucontext.h>
#include <asm/prctl.h>
#include <sys/syscall.h>

#define THR_CR_SIGNAL SIGUSR1

// This variable is needed so that the signal handler below can use the thread list.
static thread_t* background_threads;

// Declaration of thread condition variable used to pause threads during checkpoint.
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Mutex used for pausing treads during checkpointing.
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Background thread handler used to pause threads during checkpointing.
void background_threads_handler(int signum, siginfo_t* sigingo, void* ctx) {
    pid_t tid = gettid();
    void* tls;
    thread_t* thread = list_threads_find(background_threads, &tid);

    if (thread == NULL) {
        err("error: cannot register sp for background thread tid = %d\n", tid);
        return;
    }

    // Saving tls location.
    if (syscall(SYS_arch_prctl, ARCH_GET_FS, &tls)) {
        err("failed to get tls for thread tid = %d\n", tid);
    }

    if (tls != (void*) thread->cargs.tls) {
        err("thread tls was changed from %p to %p in thread tid = %d\n", thread->cargs.tls, tls, tid);
    }

    // Saving ucontext_t and fpregs.
    memcpy(&(thread->context.ctx), ctx, sizeof(ucontext_t));
    memcpy(&(thread->context.fpstate), thread->context.ctx.uc_mcontext.fpregs, sizeof(struct _libc_fpstate));
    // Acquire lock.
    pthread_mutex_lock(&lock);
    // Wait on conditional variable.
    pthread_cond_wait(&cond, &lock);
    // Release lock.
    pthread_mutex_unlock(&lock);
}

void pause_background_threads(thread_t* threads) {
    struct sigaction new_action;
    struct sigaction old_action; // TODO - save in thread_t so that we can restore.

    // Zero out the struct.
    memset(&new_action, 0, sizeof(new_action));
    memset(&old_action, 0, sizeof(old_action));

    // Initialize the global variable that keeps a copy of the threads list.
    background_threads = threads;

    // Register signal handler used to pause threads.
    new_action.sa_sigaction = background_threads_handler;
    new_action.sa_flags = SA_SIGINFO | SA_RESTART;
    sigemptyset(&new_action.sa_mask);
    sigaction(THR_CR_SIGNAL, &new_action, &old_action);

    // For each background thread, signal it.
    for (thread_t* current = threads; current != NULL; current = current->next) {
        dbg("pausing background thread tid = %d\n", *(current->tid));
        tgkill(getpid(), *(current->tid), THR_CR_SIGNAL);
    }
}

void resume_background_threads(thread_t* threads) {
    dbg("resuming background threads\n");
    pthread_cond_broadcast(&cond);
}

void init_thread(thread_t* thread, pid_t* tid, struct clone_args* cargs) {
    thread->tid = tid;
    memcpy(&(thread->cargs), cargs, sizeof(struct clone_args));
}

int list_threads_empty(thread_t* head) {
    return head->tid == NULL;
}

thread_t* list_threads_push(thread_t* head, pid_t* tid, struct clone_args* cargs) {
    thread_t* current = head;

    // If the list is empty, fill in the head and return.
    if (current->tid == NULL) {
        init_thread(current, tid, cargs);
        return current;
    }

    // Insert new element after current.
    thread_t * new = (thread_t *) malloc(sizeof(thread_t));
    memset(new, 0, sizeof(thread_t));
    init_thread(new, tid, cargs);
    new->next = current->next;
    current->next = new;
    return new;
}

void list_threads_delete(thread_t * head, thread_t* to_delete) {
    // If the list is empty or the element to delete is null, do nothing.
    if (head->tid == NULL || to_delete == NULL) {
        return;
    }
    // If the element to delete is the head, we copy the second element into the head and delete the second element.
    else if (head == to_delete) {
        if (head->next == NULL) {
            memset(head, 0, sizeof(thread_t));
        } else {
            // Save pointer to the second element.
            to_delete = head->next;
            // Copy the second element into the head.
            memcpy(head, head->next, sizeof(thread_t));
            // Free the second element.
            free(to_delete);
        }
        return;
    }
    // Else, iterate the list until you find the element to delete.
    else {
        thread_t* current = head;
        // Iterate until we find the node just before the one to delete.
        while (current != NULL) {
            // If the element to delete is the next
            if (current->next == to_delete) {
                current->next = to_delete->next;
                free(to_delete);
                return;
            }
            current = current->next;
        }
    }
    err("warning: unable to delete thread %d\n", *(to_delete->tid));
}

thread_t* list_threads_find(thread_t* head, pid_t* tid) {
    // If the list if empty, nothing to print.
    if (head->tid == NULL) {
        return NULL;
    }

    // Loop through the list until we find the corresponding tid.
    for (thread_t* current = head; current != NULL ; current = current->next) {
        if (*(current->tid) == *tid) {
            return current;
        }
    }

    // If no tid is matched, return null.
    return NULL;
}

void print_thread_cargs(struct clone_args* cargs) {
    log("thread clone args: flags = %p stack = %p tls = %p\n",
        (void*) cargs->flags, (void*) cargs->stack, (void*) cargs->tls);
}

void print_thread_context(thread_context_t* context) {
    log("thread context: rsp = %p rip = %p\n",
        (void*) context->ctx.uc_mcontext.gregs[REG_RSP],
        (void*) context->ctx.uc_mcontext.gregs[REG_RIP]);
}

void print_thread(thread_t * thread) {
    log("thread: tid = %d\n", *(thread->tid));
    print_thread_cargs(&(thread->cargs));
    print_thread_context(&(thread->context));
}

void print_list_threads(thread_t * head) {
    for (thread_t* current = head; current != NULL; current = current->next) {
        print_thread(current);
    }
}