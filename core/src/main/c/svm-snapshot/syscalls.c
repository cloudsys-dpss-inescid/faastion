// Important to load some headers such as sched.h.
#define _GNU_SOURCE

#include "syscalls.h"
#include "cr.h"

#include <errno.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <linux/sched.h>
#include <poll.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>

// Note: not defined in musl libc.
#define SECCOMP_USER_NOTIF_FLAG_CONTINUE (1UL << 0)

void print_mmap(mmap_t* sargs) {
    dbg("mmap:     %16p - %16p size = 0x%16lx prot = %s%s%s%s flags = %8d fd = %2d offset = %8ld ret = %16p\n",
        sargs->addr,
        sargs->addr == NULL ? NULL : ((char*) sargs->addr) + sargs->length,
        sargs->length,
        sargs->prot & PROT_EXEC   ? "x" : "-",
        sargs->prot & PROT_READ   ? "r" : "-",
        sargs->prot & PROT_WRITE  ? "w" : "-",
        sargs->prot == PROT_NONE  ? "n" : "-",
        sargs->flags,
        sargs->fd,
        sargs->offset,
        sargs->ret);
}

void print_munmap(munmap_t* sargs) {
    dbg("munmap:   %16p - %16p size = 0x%16lx ret  =  %d\n",
        sargs->addr,
        ((char*) sargs->addr) + sargs->length,
        sargs->length,
        sargs->ret);
}

void print_mprotect(mprotect_t* sargs) {
    dbg("mprotect: %16p - %16p size = 0x%16lx prot = %s%s%s%s ret = %d\n",
        sargs->addr,
        ((char*) sargs->addr) + sargs->length,
        sargs->length,
        sargs->prot & PROT_EXEC   ? "x" : "-",
        sargs->prot & PROT_READ   ? "r" : "-",
        sargs->prot & PROT_WRITE  ? "w" : "-",
        sargs->prot == PROT_NONE  ? "n" : "-",
        sargs->ret);
}

void print_madvise(madvise_t* sargs) {
    dbg("madvise: %16p - %16p size = 0x%16lx advice = %d ret = %d\n",
        sargs->addr,
        ((char*) sargs->addr) + sargs->length,
        sargs->length,
        sargs->advice,
        sargs->ret);
}

void print_dup(dup_t* sargs) {
    dbg("dup(%d) -> %d\n", sargs->oldfd, sargs->ret);
}

void print_dup2(dup2_t* sargs) {
    dbg("dup2(%d, %d) -> %d\n", sargs->oldfd, sargs->newfd, sargs->ret);
}

void print_open(open_t* sargs) {
    dbg("open(%s) -> %d\n", sargs->pathname, sargs->ret);
}

void print_openat(openat_t* sargs) {
    dbg("openat(%d, %s) -> %d\n", sargs->dirfd, sargs->pathname, sargs->ret);
}

void print_close(close_t* sargs) {
    dbg("close(%d) -> %d\n", sargs->fd, sargs->ret);
}

void print_exit(exit_t* sargs) {
    dbg("exit(%d)\n", sargs->pid);
}

void print_clone(struct clone_args* cargs) {
    dbg("clone (stack = %p stack_size = %llu)\n", (void*) cargs->stack, cargs->stack_size);
}

void print_clone3(struct clone_args* cargs) {
    dbg("clone3 (stack = %p stack_size = %llu)\n", (void*) cargs->stack, cargs->stack_size);
}

void handle_mmap(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, void* ret) {
    mmap_t syscall_args = {
        .addr = (void*) args[0],
        .length = (size_t) args[1],
        .prot = (int) args[2],
        .flags = (int) args[3],
        .fd = (int) args[4],
        .offset = (off_t) args[5],
        .ret = ret};
    print_mmap(&syscall_args);

    // Checkpoint the syscall.
    checkpoint_syscall(meta_snap_fd, __NR_mmap, &syscall_args, sizeof(mmap_t));

    mapping_t* mapping = list_mappings_find(mappings, ret, syscall_args.length);

    // Add mapping if it does not exist yet.
    if (mapping == NULL) {
        mapping = list_mappings_push(mappings, ret, syscall_args.length);
    }

    // Update permissions.
    mapping_update_permissions(mapping, ret, ((char*) ret + syscall_args.length), (char) syscall_args.prot);
}

void handle_munmap(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, int ret) {
    munmap_t syscall_args = {.addr = (void*) args[0], .length = (size_t) args[1], .ret = ret};
    int pagesize = getpagesize();
    print_munmap(&syscall_args);

    // Checkpoint the syscall.
    checkpoint_syscall(meta_snap_fd, __NR_munmap, &syscall_args, sizeof(munmap_t));

     if ((unsigned long)syscall_args.addr % pagesize) {
        err("warning, munmap start not a multiple of page boundary\n");
     }

    // When length is not a multiple of pagesize, we extend it to the next page boundary (check mmap man).
    if (syscall_args.length % pagesize) {
        size_t padding = (pagesize - (syscall_args.length % pagesize));
        syscall_args.length += padding;
    }

    mapping_t* mapping = list_mappings_find(mappings, syscall_args.addr, syscall_args.length);

    // Add mapping if it does not exist yet.
    if (mapping == NULL) {
        err("warning: unmapping unkown mapping %16p - %16p\n", syscall_args.addr, ((char*) syscall_args.addr) + syscall_args.length);
        return;
    }

    // Update mapping size to comply with munmap.
    mapping_update_size(mappings, mapping, syscall_args.addr, ((char*) syscall_args.addr) + syscall_args.length);
}

void handle_mprotect(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, int ret) {
    mprotect_t syscall_args = {.addr = (void*) args[0], .length = (size_t) args[1], .prot = (int) args[2], .ret = ret};
    print_mprotect(&syscall_args);

    // Checkpoint the syscall.
    checkpoint_syscall(meta_snap_fd, __NR_mprotect, &syscall_args, sizeof(mprotect_t));

    mapping_t* mapping = list_mappings_find(mappings, syscall_args.addr, syscall_args.length);

    // Add mapping if it does not exist yet.
    if (mapping == NULL) {
        err("warning: mprotecting unkown mapping %16p - %16p\n", syscall_args.addr, ((char*) syscall_args.addr) + syscall_args.length);
        return;
    }

    // Update permissions.
    mapping_update_permissions(mapping, syscall_args.addr, ((char*) syscall_args.addr + syscall_args.length), syscall_args.prot);
}

void handle_madvise(int meta_snap_fd, mapping_t* mappings, long long unsigned int* args, int ret) {
    madvise_t syscall_args = {.addr = (void*) args[0], .length = (size_t) args[1], .advice = (int) args[2], .ret = ret};
    int pagesize = getpagesize();
    print_madvise(&syscall_args);

    // Note: we do not checkpoint this syscall, it can be ignored on the restore side.

    if (syscall_args.advice != MADV_DONTNEED) {
        err("warning, madvise advice not supported: %d\n", syscall_args.advice);
    }

    if ((unsigned long)syscall_args.addr % pagesize) {
        err("warning, madvise start not a multiple of page boundary\n");
     }

    // When length is not a multiple of pagesize, we extend it to the next page boundary (check mmap man).
    if (syscall_args.length % pagesize) {
        size_t padding = (pagesize - (syscall_args.length % pagesize));
        syscall_args.length += padding;
    }

    mapping_t* mapping = list_mappings_find(mappings, syscall_args.addr, syscall_args.length);

    // Add mapping if it does not exist yet.
    if (mapping == NULL) {
        err("warning: unmapping unkown mapping %16p - %16p\n", syscall_args.addr, ((char*) syscall_args.addr) + syscall_args.length);
        return;
    }

    // TODO - some of these might be stacks. The stack allocated for the function main thread.

    // Update mapping size to comply with MADV_DONTNEED.
    mapping_update_permissions(mapping, syscall_args.addr, ((char*) syscall_args.addr + syscall_args.length), PROT_NONE);
}

void handle_dup(int meta_snap_fd, long long unsigned int* args, int ret) {
    dup_t syscall_args = {.oldfd = (int) args[0], .ret = ret};
    print_dup(&syscall_args);
    checkpoint_syscall(meta_snap_fd, __NR_dup, &syscall_args, sizeof(dup_t));
}

void handle_dup2(int meta_snap_fd, long long unsigned int* args, int ret) {
    dup2_t syscall_args = {.oldfd = (int) args[0], .newfd = (int) args[1], .ret = ret};
    print_dup2(&syscall_args);
    checkpoint_syscall(meta_snap_fd, __NR_dup2, &syscall_args, sizeof(dup2_t));
}

void handle_open(int meta_snap_fd, long long unsigned int* args, int ret) {
    open_t syscall_args = {.flags = (int) args[1], .mode = (mode_t) args[2], .ret = ret};
    char* pathname = (char*) args[0];

    // If pathname is larger than max, error out, otherwise, copy.
    if (strlen(pathname) > MAX_PATHNAME) {
        err("error, cannot checkpoint pathname longer than %d: %s\n", MAX_PATHNAME, pathname);
    } else {
        strcpy(syscall_args.pathname, pathname);
    }

    print_open(&syscall_args);
    checkpoint_syscall(meta_snap_fd, __NR_open, &syscall_args, sizeof(open_t));
}

void handle_openat(int meta_snap_fd, long long unsigned int* args, int ret) {
    openat_t syscall_args = {.dirfd = (int) args[0], .flags = (int) args[2], .mode = (mode_t) args[3], .ret = ret};
    char* pathname = (char*) args[1];

    // If pathname is larger than max, error out, otherwise, copy.
    if (strlen(pathname) > MAX_PATHNAME) {
        err("error, cannot checkpoint pathname longer than %d: %s\n", MAX_PATHNAME, pathname);
    } else {
        strcpy(syscall_args.pathname, pathname);
    }

    print_openat(&syscall_args);
    checkpoint_syscall(meta_snap_fd, __NR_openat, &syscall_args, sizeof(openat_t));
}

void handle_close(int meta_snap_fd, long long unsigned int* args, int ret) {
    close_t syscall_args = {.fd = (int) args[0], .ret = ret};
    print_close(&syscall_args);
    checkpoint_syscall(meta_snap_fd, __NR_close, &syscall_args, sizeof(close_t));
}

int should_follow_clone(int flags) {
    return flags & CLONE_VM && flags & CLONE_FS && flags & CLONE_FILES && flags & CLONE_SIGHAND && flags & CLONE_THREAD && flags & CLONE_SYSVSEM;
}

int should_follow_clone3(struct clone_args *cl_args, size_t size) {
    return should_follow_clone(cl_args->flags);
}

void handle_clone(int meta_snap_fd, thread_t* threads, long long unsigned int* args) {
    struct clone_args cargs = {0};
    cargs.flags = args[0];
    cargs.stack = args[1];
    cargs.parent_tid = args[2];
    cargs.child_tid = args[3];
    cargs.tls = args[4];
    print_clone(&cargs);
#ifdef THREADS
    list_threads_push(threads, (pid_t *) args[3], &cargs);
#endif
}

void handle_clone3(int meta_snap_fd, thread_t* threads, struct clone_args *cargs) {
    print_clone3(cargs);
#ifdef THREADS
    list_threads_push(threads, (pid_t *) cargs->child_tid, cargs);
#endif
}

void handle_exit(int meta_snap_fd, thread_t* threads, pid_t pid) {
    exit_t syscall_args = {.pid = pid};
    print_exit(&syscall_args);
#ifdef THREADS
    list_threads_delete(threads, list_threads_find(threads, (pid_t*) &pid));
#endif
}


void* allow_syscalls(void* args) {
    int seccomp_fd = *((int*) args);
    struct seccomp_notif_sizes sizes;
    if (syscall(SYS_seccomp, SECCOMP_GET_NOTIF_SIZES, 0, &sizes) < 0) {
        err("error: failed to seccomp(SECCOMP_GET_NOTIF_SIZES)");
        return NULL;
    }

    struct seccomp_notif *req = (struct seccomp_notif*)malloc(sizes.seccomp_notif);
    struct seccomp_notif_resp *resp = (struct seccomp_notif_resp*)malloc(sizes.seccomp_notif_resp);
    struct pollfd fds[1] = {
        {
            .fd  = seccomp_fd,
            .events = POLLIN,
        },
    };

    for (;;) {

        // Wait for a notification
        int events = poll(fds, 1, -1);
        if (events < 0) {
            err("error: failed to pool for events");
            continue;
        } else if (fds[0].revents & POLLNVAL) {
            break;
        } else if (events > 1) {
            err("warning: received multiple events at once!\n");
        }

        // Receive notification
        memset(req, 0, sizes.seccomp_notif);
        memset(resp, 0, sizes.seccomp_notif_resp);
        if (ioctl(seccomp_fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_RECV)");
            continue;
        }

        // Validate notification
        if (ioctl(seccomp_fd, SECCOMP_IOCTL_NOTIF_ID_VALID, &req->id) == -1 ) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
            continue;
        }

        // TODO - terminate if the number of monitored threads falls to zero.

        // Send response
        resp->id = req->id;
        resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
        if (ioctl(seccomp_fd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_SEND)");
            continue;
        }
    }
    return NULL;
}

void handle_syscalls(size_t seed, int seccomp_fd, int* finished, int meta_snap_fd, mapping_t* mappings, thread_t* threads) {
    // Base for memory mappings. Each seed value is 64GB apart (36 bits used out of 48 usable bits).
    size_t mem_base = 0xA00000000000 + 0x1000000000 * seed;
    // This number represents the number of threads that are initially running in the sandbox.
    int active_threads = 1;
    int pagesize = getpagesize();
    struct seccomp_notif_sizes sizes;
    if (syscall(SYS_seccomp, SECCOMP_GET_NOTIF_SIZES, 0, &sizes) < 0) {
        err("error: failed to seccomp(SECCOMP_GET_NOTIF_SIZES)");
        return;
    }

    struct seccomp_notif *req = (struct seccomp_notif*)malloc(sizes.seccomp_notif);
    struct seccomp_notif_resp *resp = (struct seccomp_notif_resp*)malloc(sizes.seccomp_notif_resp);
    struct pollfd fds[1] = {
        {
            .fd  = seccomp_fd,
            .events = POLLIN,
        },
    };

    while (active_threads) {

        // Wait for a notification
        int events = poll(fds, 1, 100);
        if (events < 0) {
            err("error: failed to pool for events");
            continue;
        } else if (events == 0 && *finished) {
            break;
        } else if (fds[0].revents & POLLNVAL) {
            break;
        } else if (events > 1) {
            err("warning: received multiple events at once!\n");
        }

        // Receive notification
        memset(req, 0, sizes.seccomp_notif);
        memset(resp, 0, sizes.seccomp_notif_resp);
        if (ioctl(seccomp_fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_RECV)");
            continue;
        }

        // Validate notification
        if (ioctl(seccomp_fd, SECCOMP_IOCTL_NOTIF_ID_VALID, &req->id) == -1 ) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
            continue;
        }

        // Send response
        resp->id = req->id;
        long long unsigned int *args = req->data.args;
        switch (req->data.nr) {
            case __NR_dup:
                resp->val = syscall(__NR_dup, args[0]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_dup(meta_snap_fd, args, resp->val);
                break;
            case __NR_dup2:
                resp->val = syscall(__NR_dup2, args[0], args[1]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_dup2(meta_snap_fd, args, resp->val);
                break;
            case __NR_open:
                resp->val = syscall(__NR_open, args[0], args[1], args[2]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_open(meta_snap_fd, args, resp->val);
                break;
            case __NR_openat:
                resp->val = syscall(__NR_openat, args[0], args[1], args[2], args[3]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_openat(meta_snap_fd, args, resp->val);
                break;
            case __NR_close:
                resp->val = syscall(__NR_close, args[0]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_close(meta_snap_fd, args, resp->val);
                break;
            case __NR_exit:
                handle_exit(meta_snap_fd, threads, req->pid);
                active_threads--;
                resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
                break;
            case __NR_clone:
                if (should_follow_clone((int) args[0])) {
                    handle_clone(meta_snap_fd, threads, args);
                    active_threads++;
                    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
                } else {
                    err("warning: blocking clone (new process)!\n");
                    resp->val = -1;
                    resp->error = -EAGAIN;
                    resp->flags = 0;
                }
                break;
            case __NR_clone3:
                struct clone_args* cargs = (struct clone_args*) args[0];
                if (should_follow_clone3(cargs, (size_t) args[1])) {
                    handle_clone3(meta_snap_fd, threads, cargs);
                    active_threads++;
                    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
                } else {
                    err("warning: blocking clone3 (new process)!\n");
                    resp->val = -1;
                    resp->error = -EAGAIN;
                    resp->flags = 0;
                }
                break;
            case __NR_mmap:
                // If the size is not a multiple of page size, apply padding.
                // When length is not a multiple of pagesize, we extend it to the next page boundary (check mmap man).
                if (args[1] % pagesize) {
                    size_t padding = (pagesize - (args[1] % pagesize));
                    args[1] = args[1] + padding;
                }
                // If the address is not defined, move into sandbox.
                if ((void*)args[0] == NULL && mem_base != 0) {
                    // Set top of the sandbox.
                    args[0] = mem_base;
                    // Update base for next time.
                    mem_base += args[1];
                    // Add fixed to flags to ensure new base.
                    args[3] = args[3] | MAP_FIXED;
                }

                resp->val = syscall(__NR_mmap, args[0], args[1], args[2], args[3], args[4], args[5]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_mmap(meta_snap_fd, mappings, args, (void*) resp->val);
                break;
            case __NR_munmap:
                resp->val = syscall(__NR_munmap, args[0], args[1]);
                resp->error = resp->val < 0 ? -errno: 0;
                resp->flags = 0;
                handle_munmap(meta_snap_fd, mappings, args, (int) resp->val);
                break;
            case __NR_mprotect:
                resp->val = syscall(__NR_mprotect, args[0], args[1], args[2]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_mprotect(meta_snap_fd, mappings, args , (int) resp->val);
                break;
            case __NR_madvise:
                resp->val = syscall(__NR_madvise, args[0], args[1], args[2]);
                resp->error = resp->val < 0 ? -errno : 0;
                resp->flags = 0;
                handle_madvise(meta_snap_fd, mappings, args, (int) resp->val);
                break;
            default:
                // TODO - in theory, we should be notified on all syscalls.
                err("warning: unhandled syscall %d!\n", req->data.nr);
                resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
                break;
        }

        if (ioctl(seccomp_fd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_SEND)");
            continue;
        }
    }

    free(req);
    free(resp);
}

int install_seccomp_filter() {
    struct sock_filter filter[] = {
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, arch))),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
        BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, nr))),

        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_madvise, 14, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_dup3,    13, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_dup2,    12, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_dup,     11, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_close,   10, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_creat,    9, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_openat2,  8, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_openat,   7, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_open,     6, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_exit,     5, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_clone,    4, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_clone3,   3, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_mprotect, 2, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_mmap,     1, 0),
        BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_munmap,   0, 1),
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),

        // default rule
        BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
        .len = (unsigned short)(sizeof(filter) / sizeof(filter[0])),
        .filter = filter,
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("error: failed to prctl(NO_NEW_PRIVS)");
        return -1;
    }

    int fd = syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (fd < 0) {
        perror("error: failed to seccomp(SECCOMP_SET_MODE_FILTER)");
        return -1;
    }

    return fd;
}
