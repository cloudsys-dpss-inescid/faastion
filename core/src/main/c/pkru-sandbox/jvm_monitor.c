#include "seccomp.h"
#include "hash_table.h"
#include "memory_map.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <sys/syscall.h>

static struct sock_filter filter[] = {
    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, arch))),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, nr))),
        
    // BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, sysno, 1, 0),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_gettid, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),

    // default rule
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),
};

int install_jvm_filter() {
    return install_seccomp_filter(filter);
}

static void handle_syscalls(IsolateFunction *function) {
    struct seccomp_notif *req = new_seccomp_notif();
    struct seccomp_notif_resp *resp = new_seccomp_notif_resp();
    int fd = function->notif_fd;

    for(;;) {
        if (receive_notification(fd, req, resp))
            continue;

        resp->id = req->id;
        resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
        switch (req->data.nr) {
        case __NR_gettid:
            if (hash_table_lookup(proc_tbl, req->pid) == NULL)
                hash_table_insert(proc_tbl, req->pid, function);
            break;
        default:
            fprintf(stderr, "warning: unhandled syscall %d!\n", req->data.nr);
            break;
        }

        if (send_response(fd, req, resp))
            continue;
    }

    close(fd);
    free(req);
    free(resp);
}

void* jvm_monitor(void* arg)
{
    IsolateFunction *function = (IsolateFunction *)arg;
    // Wait until seccomp_fd is set
    while (function->notif_fd == 0) ;

    handle_syscalls(function);
    return NULL;
}