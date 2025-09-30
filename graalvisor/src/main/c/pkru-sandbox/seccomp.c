#include "seccomp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <sys/syscall.h>

static struct seccomp_notif_sizes seccomp_sizes;

void seccomp_init() {
    if (syscall(SYS_seccomp, SECCOMP_GET_NOTIF_SIZES, 0, &seccomp_sizes) < 0) {
        fprintf(stderr, "error: failed to seccomp(SECCOMP_GET_NOTIF_SIZES)");
        exit(1);
    }
}

struct seccomp_notif *new_seccomp_notif() {
    return (struct seccomp_notif *)malloc(seccomp_sizes.seccomp_notif);
}

struct seccomp_notif_resp *new_seccomp_notif_resp() {
    return (struct seccomp_notif_resp *)malloc(seccomp_sizes.seccomp_notif_resp);
}

int _install_seccomp_filter(struct sock_filter filter[], int instructions)
{
    struct sock_fprog prog = {
        .len = (unsigned short)(instructions / sizeof(struct sock_filter)),
        .filter = filter,
    };

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        fprintf(stderr, "error: failed to prctl(NO_NEW_PRIVS)");
        exit(1);
    }

    int fd = syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (fd < 0) {
        fprintf(stderr, "error: failed to seccomp(SECCOMP_SET_MODE_FILTER)");
        exit(1);
    }

    return fd;
}

int receive_notification(int fd, struct seccomp_notif *req, struct seccomp_notif_resp *resp) {
    memset(req, 0, seccomp_sizes.seccomp_notif);
    if (ioctl(fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
        perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_RECV)");
        return -1;
    }
    memset(resp, 0, seccomp_sizes.seccomp_notif_resp);
    return 0;
}

int send_response(int fd, struct seccomp_notif *req, struct seccomp_notif_resp *resp) {
    if (ioctl(fd, SECCOMP_IOCTL_NOTIF_ID_VALID, &req->id) == -1 ) {
        perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
        return -1;
    }

    if (ioctl(fd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
        perror("error: failed to ioctl(SECCOMP_IOCTL_NOTIF_SEND)");
        return -1;
    }

    return 0;
}