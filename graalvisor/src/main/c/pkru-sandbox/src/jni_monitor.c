#define _GNU_SOURCE

#include "seccomp.h"
#include "memory_map.h"
#include "pkru_sandbox.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/syscall.h>

static struct sock_filter filter[] = {
    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, arch))),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),

    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, nr))),

    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_mprotect, 5, 0),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_exit, 4, 0),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_clone3, 3, 0),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_clone, 2, 0),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_munmap, 1, 0),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, __NR_mmap, 0, 1),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),

    // default rule
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_ALLOW),
};

int install_jni_filter() {
    return install_seccomp_filter(filter);
}

static void handle_syscalls(int pkey) {
    struct seccomp_notif *req = new_seccomp_notif();
    struct seccomp_notif_resp *resp = new_seccomp_notif_resp();
    int fd = monitor_threads[pkey].seccomp_fd;

    long long unsigned int *args;
    for(;;) {
        if (receive_notification(fd, req, resp))
            continue;

        resp->id = req->id;
        args = req->data.args;
        switch (req->data.nr) {
        case __NR_mmap:
            resp->val = syscall(__NR_mmap, args[0], args[1], args[2], args[3], args[4], args[5]);
            resp->error = resp->val < 0 ? -errno : 0;
            resp->flags = 0;
            if (errno == 0) {
                // fprintf(stdout, "thread id %d domain %d mmap %p-%p // %ld-%ld // prot: %d!\n",
                //     req->pid, pkey, (void *)resp->val,
                //     (void *)((char *)resp->val + (size_t)args[1]),
                //     (unsigned long)resp->val,
                //     (unsigned long)resp->val + (size_t)args[1], (int)args[2]);
                if (pkey_mprotect((void*) resp->val, (size_t) args[1], (int) args[2], pkey) == -1)
                    fprintf(stderr, "error: failed to mprotect %p for %lu bytes\n",
                        (void*) resp->val, (size_t) args[1]);
                else
                    insert_app_region(get_domain_function(pkey),
                        (void*) resp->val, (size_t) args[1], (int) args[2]);
            }                
            break;
        case __NR_munmap:
            resp->val = syscall(__NR_munmap, args[0], args[1], args[2], args[3], args[4], args[5]);
            resp->error = resp->val < 0 ? -errno : 0;
            resp->flags = 0;
            if (errno == 0)
                remove_app_region(get_domain_function(pkey), (void *)args[0], (size_t)args[1]);
            break;
        case __NR_mprotect:
            resp->val = syscall(__NR_mprotect, args[0], args[1], args[2], args[3], args[4], args[5]);
            resp->error = resp->val < 0 ? -errno : 0;
            resp->flags = 0;
            if (errno == 0)
                protect_app_region(get_domain_function(pkey),
                    (void *)args[0], (size_t)args[1], (int)args[2]);
            break;
        case __NR_clone3:
        case __NR_clone:
            clone_function_thread(get_domain_function(pkey));
            resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
            break;
        case __NR_exit:
            join_function_thread(get_domain_function(pkey));
            resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
            break;
        default:
            fprintf(stderr, "warning: unhandled syscall %d!\n", req->data.nr);
            resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
            break;
        }

        if (send_response(fd, req, resp))
            continue;
    }

    close(fd);
    free(req);
    free(resp);
}

void *jni_monitor(void* arg)
{
    int pkey = (int) ((long) arg);

    // Wait until seccomp_fd is set
    while (monitor_threads[pkey].seccomp_fd == 0) ;

    handle_syscalls(pkey);
    return NULL;
}