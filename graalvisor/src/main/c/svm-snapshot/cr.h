#ifndef CR_H
#define CR_H

#include <sys/types.h>
#include "cr_logger.h"
#include "svm-snapshot.h"
#include "list_mappings.h"
#include "list_threads.h"
#include "graal_capi.h"

// Number of fds that we allow for the function to use. We may use some fds after this limit.
#define RESERVED_FDS 768

/*
 * Limitations:
 * - after an invocation, there should be no open files or sockets, nor running threads, nor sub-processes;
 * - we assume that isolates are independent, i.e., there are no dependencies between them;
 * - we assume that libc (the only external dependency we allow) is always loaded in the same place;
 * - we do not track forked processes (clone without CLONE_VM or CLONE_FILES);
 * - functions should not expect to receive the same tid/pid after restore;
 *
 * Examples of non-supported operations:
 * - leaving a file behind when the function code does not expect that it can be left behind;
 * - mmapping file descriptors passed through unix domain sockets;
 * - forked + exeve is not tracked;
 */

// TODO - we should avoid using glibc as it may interfere with memory maps created by glibc while running the application.
// TODO - implement file descriptor optimization: keep an array with the syscall call index that openned the file descriptor of the corresponding index.

// If defined, enables debug prints and extra sanitization checks.
//#define DEBUG
// If defined, enables thread checkpointing.
#define THREADS
// If defined, enables performance measurements.
#define PERF
// If defined, enables fine-grained performance measurements.
//#define PERF_DEBUG
// If defined, enables performance optimizations.
#define OPT

#define log(format, args...) do { cr_printf(STDOUT_FILENO, format, ## args); } while(0)
#ifdef DEBUG
    #define dbg(format, args...) do { cr_printf(STDOUT_FILENO, format, ## args); } while(0)
#else
    #define dbg(format, args...) do { } while(0)
#endif
#define err(format, args...) do { cr_printf(STDERR_FILENO, format, ## args); } while(0)

// Goes through process memory maps and prints it to a file while validating our maps.
void check_proc_maps(char* filename, mapping_t * head);

// Checkpoints a single syscall invocation.
void checkpoint_syscall(int meta_snap_fd, int tag, void* syscall_args, size_t size);

// Checkpoints an svm instance memory maps.
void checkpoint(int meta_snap_fd, int mem_snap_fd, mapping_t* mappings, thread_t* threads, isolate_abi_t* abi, graal_isolate_t* isolate);

// Restores a substrace vm instance.
void restore(const char* meta_snap_path, const char* mem_snap_path, isolate_abi_t* abi, graal_isolate_t** isolate);

// Defines pid for next process to be created.
int set_next_pid(pid_t start_pid);
// Gets pid of the next process to be created.
int get_next_pid();

// Using dup (or similar syscall), moves oldfd into a reserved fd.
// Note: this function closes the oldfd.
int move_to_reserved_fd(int oldfd);

// Load function (dlopen) and look for abi symbols (dlsym),
int load_function(const char* function_path, isolate_abi_t* abi);
#endif
