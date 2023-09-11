#define _GNU_SOURCE
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include "seccomp.h"

#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof((arr)[0]))

volatile int notifyFd = 0;

static int
seccomp(unsigned int operation, unsigned int flags, void *args)
{
    return syscall(SYS_seccomp, operation, flags, args);
}

/* The following is the x86-64-specific BPF boilerplate code for checking
    that the BPF program is running on the right architecture. */

#define X86_64_CHECK_ARCH \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                (offsetof(struct seccomp_data, arch))), \
        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 1, 0), \
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL), \
        BPF_STMT(BPF_LD | BPF_W | BPF_ABS, \
                (offsetof(struct seccomp_data, nr)))


/* installNotifyFilter() installs a seccomp filter that blocks all pkey
    related system calls; the filter generates user-space notifications 
    (SECCOMP_RET_USER_NOTIF) on all other system calls.

    The function return value is a file descriptor from which the
    user-space notifications can be fetched. */

static int
installNotifyFilter(void)
{    
    struct sock_filter filter[] = {
        X86_64_CHECK_ARCH,

        /* pkey_*(2) triggers KILL signal */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_mprotect, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_alloc, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_pkey_free, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL),

        /* mmap(2) and clone(2) trigger notifications to user-space supervisor */

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_mmap, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_clone3, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

        BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_exit, 0, 1),
        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_USER_NOTIF),

        /* Every other system call is allowed */

        BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
    };

    struct sock_fprog prog = {
        .len = ARRAY_SIZE(filter),
        .filter = filter,
    };

    /* Install the filter with the SECCOMP_FILTER_FLAG_NEW_LISTENER flag;
        as a result, seccomp() returns a notification file descriptor. */

    int nfd = seccomp(SECCOMP_SET_MODE_FILTER,
                        SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (nfd == -1)
        err(EXIT_FAILURE, "seccomp-install-notify-filter");

    return nfd;
}


/* Create a child thread--the "target"--that makes system calls
    to be handled by the supervisor thread. */

static void *
target(void *arg)
{       
    /* Install seccomp filter(s) */

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
        err(EXIT_FAILURE, "prctl");

    notifyFd = installNotifyFilter();
    
    void *mapped_mem = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (mapped_mem == MAP_FAILED)
        perror("mmap");
    else
        printf("[T]: SUCCESS: mmap() returned %p\n", mapped_mem);

    return NULL;
}

/* Check that the notification ID provided by a SECCOMP_IOCTL_NOTIF_RECV
    operation is still valid. It will no longer be valid if the target
    process has terminated or is no longer blocked in the system call that
    generated the notification (because it was interrupted by a signal).

    This operation can be used when doing such things as accessing
    /proc/PID files in the target process in order to avoid TOCTOU race
    conditions where the PID that is returned by SECCOMP_IOCTL_NOTIF_RECV
    terminates and is reused by another process. */

static bool
cookieIsValid(int notifyFd, uint64_t id)
{
    return ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_ID_VALID, &id) == 0;
}

/* Allocate buffers for the seccomp user-space notification request and
    response structures. It is the caller's responsibility to free the
    buffers returned via 'req' and 'resp'. */

static void
allocSeccompNotifBuffers(struct seccomp_notif **req,
                        struct seccomp_notif_resp **resp,
                        struct seccomp_notif_sizes *sizes)
{
    size_t  resp_size;

    /* Discover the sizes of the structures that are used to receive
        notifications and send notification responses, and allocate
        buffers of those sizes. */

    if (seccomp(SECCOMP_GET_NOTIF_SIZES, 0, sizes) == -1)
        err(EXIT_FAILURE, "[S]: seccomp-SECCOMP_GET_NOTIF_SIZES");

    *req = malloc(sizes->seccomp_notif);
    if (*req == NULL)
        err(EXIT_FAILURE, "[S]: malloc-seccomp_notif");

    /* When allocating the response buffer, we must allow for the fact
        that the user-space binary may have been built with user-space
        headers where 'struct seccomp_notif_resp' is bigger than the
        response buffer expected by the (older) kernel. Therefore, we
        allocate a buffer that is the maximum of the two sizes. This
        ensures that if the supervisor places bytes into the response
        structure that are past the response size that the kernel expects,
        then the supervisor is not touching an invalid memory location. */

    resp_size = sizes->seccomp_notif_resp;
    if (sizeof(struct seccomp_notif_resp) > resp_size)
        resp_size = sizeof(struct seccomp_notif_resp);

    *resp = malloc(resp_size);
    if (*resp == NULL)
        err(EXIT_FAILURE, "[S]: malloc-seccomp_notif_resp");

}

static void
handleMmap(struct seccomp_notif *req, struct seccomp_notif_resp *resp) 
{
    SECC_DBM("\t----mmap syscall----");

    void *mapped_mem = mmap((void *)req->data.args[0], req->data.args[1],
                     req->data.args[2], req->data.args[3],
                     req->data.args[4], req->data.args[5]);

    if (mapped_mem == MAP_FAILED) {
        /* If mmap() failed in the supervisor, pass the error
            back to the target */

        resp->error = -errno;
        printf("\t[S]: failure! (errno = %d; %s)\n", errno,
                strerror(errno));
    }
    else {
        resp->error = 0;            /* "Success" */
        resp->val = (__s64)mapped_mem;   /* return value of
                                        mmap() in target */
        printf("\t[S]: success! spoofed return = %p; spoofed val = %lld\n",
                mapped_mem, resp->val);
    }
    //int domain = ERIM_EXEC_DOMAIN(__rdpkru());
    //if (pkey_mprotect((void *)req->data.args[0], (size_t)req->data.args[1],
    //                  (int)req->data.args[2], domain) == -1) {
    //    perror("pkey_mprotect");
    //}
}

static void 
handleClone(struct seccomp_notif *req, struct seccomp_notif_resp *resp)
{
    SECC_DBM("\t----clone syscall----");
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;

}

static void 
handleExit(struct seccomp_notif *req, struct seccomp_notif_resp *resp)
{
    SECC_DBM("\t----exit syscall----");
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
}

/* Handle notifications that arrive via the SECCOMP_RET_USER_NOTIF file
    descriptor, 'notifyFd'. */

static void
handleNotifications(int notifyFd)
{
    struct seccomp_notif        *req;
    struct seccomp_notif_resp   *resp;
    struct seccomp_notif_sizes  sizes;

    allocSeccompNotifBuffers(&req, &resp, &sizes);

    int nthreads = 1;
    
    /* Loop handling notifications */

    for (;;) {

        /* Wait for next notification, returning info in '*req' */

        memset(req, 0, sizes.seccomp_notif);
        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            if (errno == EINTR)
                continue;
            err(EXIT_FAILURE, "\t[S]: ioctl-SECCOMP_IOCTL_NOTIF_RECV");
        }

        SECC_DBM("\t[S]: received notifaction id [%lld], from tid: %d, syscall nr: %d", 
                req->id, req->pid, req->data.nr);

        if (!cookieIsValid(notifyFd, req->id)) {
            perror("ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
            continue;
        }

        /* Prepopulate some fields of the response */

        resp->id = req->id;     /* Response includes notification ID */
        resp->flags = 0;
        resp->val = 0;

        // Handle specific syscalls
        switch(req->data.nr) {
            case __NR_mmap:
                handleMmap(req, resp);
                break;
            case __NR_clone3:
                nthreads++;
                handleClone(req, resp);
                break;
            case __NR_exit:
                nthreads--;
                handleExit(req, resp);
            default:
                resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
                break;
        }

        /* Send a response to the notification */

        SECC_DBM("\t[S]: sending response "
                "(flags = %#x; val = %lld; error = %d)",
                resp->flags, resp->val, resp->error);

        if (ioctl(notifyFd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            if (errno == ENOENT)
                SECC_DBM("\t[S]: response failed with ENOENT; "
                        "perhaps target process's syscall was "
                        "interrupted by a signal?\n");
            else
                perror("ioctl-SECCOMP_IOCTL_NOTIF_SEND");
        }
        SECC_DBM("\t--------------------\n");

        if (!nthreads)
            break;
    }

    free(req);
    free(resp);
    SECC_DBM("\t[S]: terminating **********\n");
}

/* Implementation of the supervisor thread:

    (1) obtains the notification file descriptor
    (2) handles notifications that arrive on that file descriptor. */

static void
supervisor()
{
    while (notifyFd == 0);
    handleNotifications(notifyFd);
}

int
main()
{
    pthread_t worker;

    /* Create a child thread */
    
    pthread_create(&worker, NULL, target, NULL);

    /* Supervise child */

    supervisor();

    exit(EXIT_SUCCESS);
}
