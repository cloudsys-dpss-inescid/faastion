#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mount.h>

void init_entropy_pool();

int main(int argc, char** argv, char** envp) {

    if (!mount("none", "/proc", "proc", 0, NULL)) {
        fprintf(stderr, "succeeded to mount!");
    } else {
        perror("mount");
        exit(EXIT_FAILURE);
    }

    init_entropy_pool();

    if (!chdir("/nodejsAction")) {
        fprintf(stderr, "succeeded to change directory!");
    } else {
        perror("chdir");
        exit(EXIT_FAILURE);
    }

    char *newargv[] = { "/usr/local/bin/node", "--expose-gc", "app.js", NULL };
    char *newenvp[] = { "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 
                        "NODE_VERSION=12.22.2",
                        "YARN_VERSION=1.22.5", NULL };
    execve("/usr/local/bin/node", newargv, newenvp);
    perror("execve");
    exit(EXIT_FAILURE);
}
