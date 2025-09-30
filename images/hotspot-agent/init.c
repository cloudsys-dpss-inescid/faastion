#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include<sys/mount.h>


int main(int argc, char** argv, char** envp) {

    if (!mount("none", "/proc", "proc", 0, NULL)) {
        printf("succeeded to mount!");
    } else {
        perror("mount");
        exit(EXIT_FAILURE);
    }

    char *newargv[] = { "/jvm/bin/java",
                            "-Djava.library.path=/jvm/lib",
                            "-agentlib:native-image-agent=config-merge-dir=config,caller-filter-file=caller-filter-config.json,config-write-initial-delay-secs=90,config-write-period-secs=60",
                            "-cp", "graalvisor-1.0-all.jar",
                            "org.graalvm.argo.graalvisor.Main", NULL };
    char *newenvp[] = { "JAVA_HOME=/jvm", NULL };
    execve("/jvm/bin/java", newargv, newenvp);
    perror("execve");
    exit(EXIT_FAILURE);
}
