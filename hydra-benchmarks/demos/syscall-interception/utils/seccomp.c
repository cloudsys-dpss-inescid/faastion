#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <linux/seccomp.h>
#include <linux/filter.h>
#include <linux/audit.h>
#include "seccomp.h"

#define MODE_STRICT_STRING "--mode-strict"
#define MODE_FS_STRING "--mode-fs" 
#define MODE_NET_STRING "--mode-net"
#define MODE_ALLOW_RW_STRING "--mode-allow-rw"

#define FILE_SIZE 256

typedef enum seccomp_mode_enum { MODE_STRICT, MODE_ALLOW_RW } seccomp_mode;

int target_pid;
volatile int nfd = 0;
char pathname[FILE_SIZE];

char *executable;
char **args;
int mode;

unsigned long intercept_count = 0;

struct sock_filter strict_filter[] = {
    BPF_STMT(BPF_LD + BPF_W + BPF_ABS, (offsetof(struct seccomp_data, arch))),
    BPF_JUMP(BPF_JMP + BPF_JEQ + BPF_K, AUDIT_ARCH_X86_64, 1, 0),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_KILL),
    BPF_STMT(BPF_RET + BPF_K, SECCOMP_RET_USER_NOTIF),
};

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



// Prototypes

void sigchld_handler(int signum);
void sigterm_handler(int signum);
int install_notify_filter(int mode);
void handle_notifications(int nfd);
int intercept_syscalls();
void seccomp_init();

// Definitions

int is_reg_file(const char *file) {
    static struct stat sb;
    return stat(file, &sb) == 0 && S_ISREG(sb.st_mode);
}

char *_get_pathname(char name[], char *dir) {
    if (dir == NULL)
        return NULL;
    snprintf(pathname, FILE_SIZE-1, "%s/%s", dir, name);
    return is_reg_file(pathname) ? pathname : _get_pathname(name, strtok(NULL, ":"));
}

char *get_pathname(char name[]) {
    char *path = getenv("PATH");
    if (is_reg_file(name)) {
        strncpy(pathname, name, FILE_SIZE-1);
        return pathname;
    }
    else if (path == NULL || strlen(path) <= 0)
        return NULL;
    else
        return _get_pathname(name, strtok(path, ":"));
}

int get_seccomp_mode(char *mode) {
    if (strncmp(mode, MODE_STRICT_STRING, sizeof(MODE_STRICT_STRING)) == 0)
        return MODE_STRICT;
    else if (strncmp(mode, MODE_ALLOW_RW_STRING, sizeof(MODE_ALLOW_RW_STRING)) == 0)
        return MODE_ALLOW_RW;
    else
        return -1;
}

void seccomp_init() {
    struct sigaction sigact;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigact.sa_handler = sigchld_handler;
    sigaction(SIGCHLD, &sigact, NULL);
    sigact.sa_handler = sigterm_handler;
    sigaction(SIGTERM, &sigact, NULL);
}

void *run_command(void *arg) {
    nfd = install_notify_filter(mode);
    if (fork() == 0)
        execv(pathname, args);
    return NULL;
}

int exec(char pahtname[], char *argv[], int seccomp_mode) {
    pthread_t worker;
    executable = pahtname;
    args = argv;
    mode = seccomp_mode;
    seccomp_init();
    pthread_create(&worker, NULL, run_command, NULL);
    intercept_syscalls();
    return 0;
}

int intercept_syscalls() {
    while (nfd == 0) ;
    handle_notifications(nfd);
    return 0;
}

int install_notify_filter(int mode) {
    struct sock_fprog prog;
    switch (mode) {
        case MODE_STRICT:
            prog.len = (unsigned short)(sizeof(strict_filter) / sizeof(strict_filter[0]));
            prog.filter = strict_filter;
            break;
        case MODE_ALLOW_RW:
            prog.len = (unsigned short)(sizeof(allow_rw_filter) / sizeof(allow_rw_filter[0]));
            prog.filter = allow_rw_filter;
            break;
        default:
            break;
    }

    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0)) {
        perror("prctl(NO_NEW_PRIVS)");
        return -1;
    }
    int rc = syscall(SYS_seccomp, SECCOMP_SET_MODE_FILTER, SECCOMP_FILTER_FLAG_NEW_LISTENER, &prog);
    if (rc < 0) {
        perror("seccomp(SECCOMP_SET_MODE_FILTER)");
        return -1;
    }
    else {
        return rc;
    }
}

void log_syscalls() {
    char buf[22];
    FILE *f = fopen("seccomp.out", "w");
    sprintf(buf, "%lu\n", intercept_count);
    fwrite(buf, sizeof(char), strlen(buf), f);
    fclose(f);
}

void sigchld_handler(int signum) {
    log_syscalls();
    exit(0);
}

void sigterm_handler(int signum) {
    kill(target_pid, SIGKILL);
    log_syscalls();
    exit(0);
}

void handle_notifications(int nfd) {
    struct seccomp_notif_sizes sizes;
    if (syscall(SYS_seccomp, SECCOMP_GET_NOTIF_SIZES, 0, &sizes) < 0) {
        perror("seccomp(SECCOMP_GET_NOTIF_SIZES)");
        return;
    }

    struct seccomp_notif *req = (struct seccomp_notif*)malloc(sizes.seccomp_notif);
    struct seccomp_notif_resp *resp = (struct seccomp_notif_resp*)malloc(sizes.seccomp_notif_resp);

    while (1) {
        // Receive notification
        memset(req, 0, sizes.seccomp_notif);
        memset(resp, 0, sizes.seccomp_notif_resp);
        if (ioctl(nfd, SECCOMP_IOCTL_NOTIF_RECV, req) == -1) {
            perror("ioctl(SECCOMP_IOCTL_NOTIF_RECV)");
            continue;
        }

        // Print notification
        intercept_count++;
#ifdef DEBUG
        fprintf(stderr, "Received notifaction id [%lld], from tid: %d, syscall nr: %d\n", req->id, req->pid, req->data.nr);
#endif

        // Validate notification
        if (ioctl(nfd, SECCOMP_IOCTL_NOTIF_ID_VALID, &req->id) == -1 ) {
            perror("ioctl(SECCOMP_IOCTL_NOTIF_ID_VALID)");
            continue;
        }

        // Send response
        resp->id = req->id;
        resp->flags = SECCOMP_USER_NOTIF_FLAG_CONTINUE;
        if (ioctl(nfd, SECCOMP_IOCTL_NOTIF_SEND, resp) == -1) {
            perror("ioctl(SECCOMP_IOCTL_NOTIF_SEND)");
            continue;
        }
    }
}

