#ifndef SVM_SNAPSHOT_H
#define SVM_SHAPSHOT_H

// Maximum number of characters to receive from a function invocation.
#define FOUT_LEN 256

/*  Sandbox for executing entrypoints and getting their results. */
typedef struct svm_sandbox_t        svm_sandbox_t;
typedef struct forked_svm_sandbox_t forked_svm_sandbox_t;

/*  Executes entrypoint from the provided sandbox. */
void invoke_svm(
    // Sandbox for executing entrypoints and getting their results.
    svm_sandbox_t* sandbox,
    // Arguments passed to the function upon each invocation.
    // Expects null-terminated string with max len FOUT_LEN
    const char* fin,
    // Output buffer where the output of the invocation will be placed.
    // Note that if multiple invocations are performed (as a result of concurrency
    // or requests), only the output of the first request will be saved.
    // If the output string is larger than FOUT_LEN, a warning
    // is printed to stdout and a '\0' is placed at outbuf[outbuf_len - 1].
    char* fout);

/* Similar to the non-forked version. */
void forked_invoke_svm(
    forked_svm_sandbox_t* sandbox,
    const char* fin,
    char* fout);

/* Destroys an svm sandbox. If `rease_isolate` is non-zero, then the isolate used
   in this sandbox is deleted and the thread handling requests for this sandbox is
   terminated. Otherwise, the isolate is kept alive and the thread terminates.
*/
void destroy_svm(
    // Sandbox to be destroyed.
    svm_sandbox_t* sandbox,
    // If zero, the isolate used in this sandbox will be terminated.
    int reuse_isolate);

/* Similar to non-forked version. */
void forked_destroy_svm(forked_svm_sandbox_t* sandbox, int reuse_isolate);

/* Creates a new svm_sandbox_t pointing to the same underlying sandbox. A cloned
   sandbox can be invoked concurrently with other clones and the original. */
svm_sandbox_t* clone_svm(
    // Sandbox for executing entrypoints and getting their results.
    svm_sandbox_t* sandbox,
    // If zero, a new isolate will be created for this sandbox.
    int reuse_isolate);

/* Similar to the non-forked version. */
forked_svm_sandbox_t* forked_clone_svm(
    forked_svm_sandbox_t* sandbox,
    int reuse_isolate);

/*  Loads, runs and then checkpoints a substrate vm instance.
    Returns svm_sandbox_t to be able to invoke more runs. */
svm_sandbox_t* checkpoint_svm(
    // Path of the function library that will be dlopened.
    const char* fpath,
    // Path where to store metadata information.
    const char* meta_snap_path,
    // Path where to store memory dumps.
    const char* mem_snap_path,
    // The seed is used to control which virtual memory ranges the svm instance
    // will used. Each seed value represents a 16TB virtual memory range. When
    // calling restore, the user must make sure there is no restoredsvm instance
    // using the same range.
    unsigned long seed,
    // Number of concurrent threads that will invoke the function code.
    unsigned int concurrency,
    // Number of invocations each thread will perform.
    unsigned int requests,
    // Arguments passed to the function upon each invocation.
    // Expects null-terminated string with max len FOUT_LEN
    const char* fin,
    // Output buffer where the output of the invocation will be placed.
    // Note that if multiple invocations are performed (as a result of concurrency
    // or requests), only the output of the first request will be saved.
    // If the output string is larger than FOUT_LEN, a warning
    // is printed to stdout and a '\0' is placed at outbuf[outbuf_len - 1].
    char* fout);

/*  Loads a checkpointed substrate vm instance and then runs it.
    Returns svm_sandbox_t to be able to invoke more runs. */
svm_sandbox_t* restore_svm(
    // Path of the function library that will be dlopened.
    const char* fpath,
    // Path where to store metadata information.
    const char* meta_snap_path,
    // Path where to store memory dumps.
    const char* mem_snap_path,
    // The seed is used to control which virtual memory ranges the svm instance
    // will used. Each seed value represents a 16TB virtual memory range. When
    // calling restore, the user must make sure there is no restoredsvm instance
    // using the same range.
    unsigned long seed);

/* Similar to the non-forked version. */
forked_svm_sandbox_t* forked_restore_svm(
    const char* fpath,
    const char* meta_snap_path,
    const char* mem_snap_path,
    unsigned long seed);

/* Unloads a function/snapshot. */
void forked_unload_svm(
    // Sandbox to be destroyed.
    forked_svm_sandbox_t* sandbox);

/*  Loads and runs a substrate vm instance. */
void run_svm(
    // Path of the function library that will be dlopened.
    const char* fpath,
    // Number of concurrent threads that will invoke the function code.
    unsigned int concurrency,
    // Number of invocations each thread will perform.
    unsigned int requests,
    // Arguments passed to the function upon each invocation.
    // Expects null-terminated string with max len FOUT_LEN
    const char* fin,
    // Output buffer where the output of the invocation will be placed.
    // Note that if multiple invocations are performed (as a result of concurrency
    // or requests), only the output of the first request will be saved.
    // If the output string is larger than FOUT_LEN, a warning
    // is printed to stdout and a '\0' is placed at outbuf[outbuf_len - 1].
    char* fout);

/*
minimize_syscalls optimizes system calls by filtering the system calls that are
safe to remove.

Currently we are optimzing open/close and mmap/munmap relations, the optimization
logic works as follows:

If a close() is issued to a fd returned by a previous open(), and there weren't
any relevant uses of the fd such as mmap(), then we can remove both system calls.
Because we don't checkpoint socket() sometimes close() is issued on an unexisting
fd, so this system call can also be removed. dup() and dup2() in our context are
functionally equivalent to open() because they return an fd.

If an munmap() is issued to a previously seen returned address from an mmap(), we
check if the address was seen previously, and if so, compares the length. mmap()
returns a page-aligned length due to our seccomp intercepter, so we also add
padding to munmap() length so that it becomes page-aligned and can be compared
correctly with the length of the mmap().
*/

/*  Loads syscalls saved in meta_snap_path file, removes unnecessary ones and saves
    output to output_path.
    If same path is provided the old meta_snap_path will be overwritten. */
void minimize_syscalls(
    // Path where metadata information is stored.
    const char* meta_snap_path,
    // Path where to store metadata information.
    const char* output_path);

#endif