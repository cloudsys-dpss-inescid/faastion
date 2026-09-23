#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "list_mappings.h"
#include "list_threads.h"

#include <fcntl.h>
#include <stdio.h>

#define MAX_PATHNAME 255

// Structs used to serialize system call arguments.
typedef struct { void* addr; size_t length; int prot; int flags; int fd; off_t offset; void* ret; } mmap_t;
typedef struct { void* addr; size_t length; int ret; } munmap_t;
typedef struct { void* addr; size_t length; int prot; int ret; } mprotect_t;
typedef struct { void* addr; size_t length; int advice; int ret; } madvise_t;
typedef struct { int oldfd; int ret; } dup_t;
typedef struct { int oldfd; int newfd; int ret; } dup2_t;
typedef struct { char pathname[MAX_PATHNAME]; int flags; mode_t mode; int ret; } open_t;
typedef struct { int dirfd; char pathname[MAX_PATHNAME]; int flags; mode_t mode; int ret; } openat_t;
typedef struct { int fd; int ret; } close_t;
typedef struct { int pid; } exit_t;

// Debug prings for system calls.
void print_mmap(mmap_t* syscall_args);
void print_munmap(munmap_t* syscall_args);
void print_mprotect(mprotect_t* syscall_args);
void print_madvise(madvise_t* syscall_args);
void print_dup(dup_t* syscall_args);
void print_dup2(dup2_t* syscall_args);
void print_open(open_t* syscall_args);
void print_openat(openat_t* syscall_args);
void print_close(close_t* syscall_args);
void print_exit(exit_t* syscall_args);

// Functions to deal with each individual syscall.
void handle_mmap(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, void* ret);
void handle_munmap(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, int ret);
void handle_mprotect(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, int ret);
void handle_madvise(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, int ret);
void handle_dup(int meta_snap_fd, long long unsigned int* args, int ret);
void handle_dup2(int meta_snap_fd, long long unsigned int* args, int ret);
void handle_open(int meta_snap_fd, long long unsigned int* args, int ret);
void handle_openat(int meta_snap_fd, long long unsigned int* args, int ret);
void handle_close(int meta_snap_fd, long long unsigned int* args, int ret);
void handle_exit(int meta_snap_fd, thread_t* threads, pid_t pid);

// Function that allows all system calls to continue.
void* allow_syscalls(void* args);

// Function that listens to seccomp notifications an invokes syscalls.
void handle_syscalls(size_t seed, int seccomp_fd, int* finished, int meta_snap_fd, mapping_t* mappings, thread_t* threads);

// Installs seccomp filter that delives notifications handled in handle_syscalls.
int install_seccomp_filter();

#endif