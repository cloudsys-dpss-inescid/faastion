#ifndef __LAZYISOLATION_H__
#define __LAZYISOLATION_H__


/**
 * Initialize lazy isolation internal state
 * 
 * This function initializes the supervisor manager and the internal
 * shared queue. It should be called before any other functions in this library
 * to ensure correct monitoring of system calls.
*/
void initialize_seccomp();

/**
 * Installs a syscall filter in the kernel
 * 
 * This function installs a seccomp filter (blocks every system call except read/write) in the kernel.
 * This function can be called after creating a thread. 
 * Use `install_proc_filter` for new processes.
 * If used for processes, then the caller will block indefinetly on the next call to a blocked system call. 
 * If the filter fails then the thread continues to execute without the filter.
*/
void install_thread_filter();

/**
 * Installs a syscall filter in the kernel
 * @param child_pipe used to communicate with the parent process
 * 
 * This function installs a seccomp filter (blocks every system call except read/write) in the kernel.
 * This function can be called after a fork to imediatelly start monitoring the newly create process.
 * If the filter fails then the process continues to execute without the filter.
*/
void install_proc_filter(int child_pipe[]);

/**
 * Attaches a child process to a supervisor
 * @param pid the process ID of the child to be monitored
 * @param child_pipe used to communicate with the child process
 * @param parent_pipe used to signal the supervisor of errors in the child process
 * 
 * This function creates a copy of the notification file descriptor and assigns the child
 * process to a new supervisor.
*/
void attach(pid_t pid, int child_pipe[], int parent_pipe[]);

#endif