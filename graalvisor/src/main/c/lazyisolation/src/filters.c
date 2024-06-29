#include <unistd.h>
#include <stddef.h>
#include <sys/syscall.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include "filters.h"

struct sock_filter allow_rw_filter[] = {
    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, arch))),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, nr))),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_read, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_readv, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_pread64, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_preadv, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_write, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_writev, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_pwrite64, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_pwritev, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_lseek, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_sendfile, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),

    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),
};

struct sock_fprog allow_rw_prog = {
    .len = (unsigned short)(sizeof(allow_rw_filter) / sizeof(allow_rw_filter[0])),
    .filter = allow_rw_filter,
};

struct sock_fprog *get_filter(int filter) {
    switch (filter) {
        case ALLOW_RW:
            return &allow_rw_prog;
        default:
            return NULL;
    }
}