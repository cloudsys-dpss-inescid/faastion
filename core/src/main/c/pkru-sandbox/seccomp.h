#ifndef __SECCOMP_H__
#define __SECCOMP_H__

#include <linux/seccomp.h>
#include <linux/bpf.h>
#include <linux/audit.h>
#include <linux/filter.h>

#include <sys/ptrace.h>

void seccomp_init();
struct seccomp_notif *new_seccomp_notif();
struct seccomp_notif_resp *new_seccomp_notif_resp();
int receive_notification(int fd, struct seccomp_notif *req, struct seccomp_notif_resp *resp);
int send_response(int fd, struct seccomp_notif *req, struct seccomp_notif_resp *resp);
int _install_seccomp_filter(struct sock_filter filter[], int instructions);
#define install_seccomp_filter(filter) _install_seccomp_filter(filter, sizeof(filter))

#endif // __SECCOMP_H__