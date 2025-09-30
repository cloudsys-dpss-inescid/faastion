#ifndef LIST_THREADS_H
#define LIST_THREADS_H
#include <linux/sched.h>
#include <sys/types.h>
#include <ucontext.h>

typedef struct thread_context {
    // fpstate structed pointed by mcontext. Note: this is the first item in the struct as it is
    // sentive to being aligned.
    struct _libc_fpstate fpstate;

    // Thread context when checkpointing.
    ucontext_t ctx;
} thread_context_t;

// Basic entry of a thread list.
// This list is used to keep track of child threads that should be checkpoint/restored.
typedef struct thread {
    // Pid of the target thread.
    pid_t* tid;

    // clone3 (and clone) arguments:
    struct clone_args cargs;

    // Thread context includes a tls and ucontext.
    thread_context_t context;

    // Pointer to the next list entry.
    struct thread* next;
} thread_t;

// Function that pauses background threads before checkpointing.
void pause_background_threads(thread_t* head);

// Function that resumes background threads after checkpointing.
void resume_background_threads(thread_t* head);

// Adds a new thread to the list of threads.
thread_t* list_threads_push(thread_t* head, pid_t* tid, struct clone_args* cargs);

// Deletes one element from the list.
void list_threads_delete(thread_t * head, thread_t* to_delete);

// Returns the list element that matches the given tid.
thread_t* list_threads_find(thread_t* head, pid_t* tid);

// Returns true (!= 0) is thead list is empty.
int list_threads_empty(thread_t* head);

// Prints a single thread or the list of threads starting from `head`.
void print_thread_cargs(struct clone_args* cargs);
void print_thread_context(thread_context_t* context);
void print_thread(thread_t * thread);
void print_list_threads(thread_t * head);

#endif