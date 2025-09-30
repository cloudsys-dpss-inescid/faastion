#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mount.h>


int main(int argc, char** argv, char** envp) {

    if (!mount("none", "/proc", "proc", 0, NULL)) {
        printf("succeeded to mount!");
    } else {
        perror("mount");
        exit(EXIT_FAILURE);
    }

    char *newargv[] = { "polyglot-proxy", NULL };
    execve("./polyglot-proxy", newargv, envp);
    perror("execve");
    exit(EXIT_FAILURE);
}
