#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include "filters.h"
#include "shared_queue.h"
#include "lazyisolation.h"

// #define DEBUG

shared_queue queue;
pthread_t supervisor_manager;
struct seccomp_notif_sizes sizes;

void *start_supervisor_manager(void *arg);
void *start_supervisor(void *arg);

/* NOTE - function header comment in lazyisolation.h */
void initialize_seccomp() {
    shared_queue_init(&queue);
    pthread_create(&supervisor_manager, NULL, start_supervisor_manager, NULL);
}

void *start_supervisor_manager(void *arg) {
    pthread_t thread;

    if (syscall(SYS_seccomp, SECCOMP_GET_NOTIF_SIZES, 0, &sizes) < 0) {
        perror("seccomp(SECCOMP_GET_NOTIF_SIZES)");
        return NULL;
    }

    while (1) {
        queue_wait(&queue);
#ifdef DEBUG
        fprintf(stderr, "Created supervisor\n");
#endif        
        pthread_create(&thread, NULL, start_supervisor, NULL);
        pthread_detach(thread);
    }

    return NULL;
}

int receive_notification(int fd, struct seccomp_notif *req) {
    memset(req, 0, sizes.seccomp_notif);
    if (ioctl(fd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
        perror("ioctl(SECCOMP_IOCTL_NOTIF_RECV)");
        return -1;
    }

#ifdef DEBUG
        fprintf(stderr, "[notify fd %d] syscall nr: %d\n", fd, req->data.nr);
#endif

    return 0;
}

int send_response(int fd, struct seccomp_notif *req, struct seccomp_notif_resp *resp) {
    // Validate notification
    if (ioctl(fd, SECCOMP_IOCTL_NOTIF_ID_VALID, &req->id) == -1 ) {
        perror("ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
        return -1;
    }

    // Send response
    memset(resp, 0, sizes.seccomp_notif_resp);
    resp->id = req->id;
    resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
    if (ioctl(fd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
        perror("ioctl(SECCOMP_IOCTL_NOTIF_SEND)");
        return -1;
    }
    return 0;
}

void handle_notifications(struct tfd *target) {
    struct seccomp_notif *req = (struct seccomp_notif*)malloc(sizes.seccomp_notif);
    struct seccomp_notif_resp *resp = (struct seccomp_notif_resp*)malloc(sizes.seccomp_notif_resp);
    struct pollfd pfds[] = {
        {
            .fd = target->nfd,
            .events = POLLIN,
        },
        {
            .fd = target->sfd,
        },
    };
    while (1) {
        if (poll(pfds, 2, -1) < 0) {
            perror("poll");
            return;
        }

        if (pfds[0].revents != 0) {
            if (pfds[0].revents & POLLIN) {
                if (receive_notification(pfds[0].fd, req))
                    continue;
                
                if (send_response(pfds[0].fd, req, resp))
                    continue;
            }
            else {
#ifdef DEBUG
                fprintf(
                    stderr,
                    "%s%s%s\n",
                    ((pfds[0].revents & POLLERR) ? "POLLERR " : ""),
                    ((pfds[0].revents & POLLHUP) ? "POLLHUP " : ""),
                    ((pfds[0].revents & POLLNVAL) ? "POLLNVAL " : "")
                );
#endif
                break;
            }
        }

        if (pfds[1].revents != 0) {
            break;
        }
    }
    free(req);
    free(resp);
#ifdef DEBUG
    fprintf(stderr, "Destroyed supervisor\n");
#endif
}

void destroy_target(struct tfd *target) {
    close(target->nfd);
    if (target->sfd >= 0)
        close(target->sfd);
}

void *start_supervisor(void *arg) {
    struct tfd target;
    if (head(&queue, &target)) { 
        fprintf(stderr, "Queue is empty\n");
        return NULL;
    }
    handle_notifications(&target);
    destroy_target(&target);
    return NULL;
}

int install_notify_filter() {
    struct sock_fprog *prog = get_filter(ALLOW_RW);
    if (prog == NULL) {
        errno = EINVAL;
        perror("get_filter");
        return -1;
    }

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl(NO_NEW_PRIVS)");
        return -1;
    }
    int rc = syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_NEW_LISTENER, prog);
    if (rc < 0) {
        perror("seccomp(SECCOMP_SET_MODE_FILTER)");
        return -1;
    }
    else {
        return rc;
    }
}

void *attach_thread(void *arg) {
    while (*(volatile int*)arg == 0) ;
    put(&queue, *(volatile int*)arg, -1);
    return NULL;
}

/* NOTE - function header comment in lazyisolation.h */
void install_thread_filter() {
    pthread_t thread;
    volatile int nfd = 0;
    pthread_create(&thread, NULL, attach_thread, (void*)&nfd);
    nfd = install_notify_filter();
    pthread_join(thread, NULL);
}

/* NOTE - function header comment in lazyisolation.h */
void install_proc_filter(int child_pipe[]) {
    int nfd = install_notify_filter();
    write(child_pipe[1], &nfd, sizeof(nfd));
}

/* NOTE - function header comment in lazyisolation.h */
void attach(pid_t pid, int child_pipe[], int parent_pipe[]) {
    int pidfd, fd;
    read(child_pipe[0], &fd, sizeof(fd));
    pidfd = syscall(__NR_pidfd_open, pid, 0);
    fd = syscall(__NR_pidfd_getfd, pidfd, fd, 0);
    put(&queue, fd, parent_pipe[0]);
}