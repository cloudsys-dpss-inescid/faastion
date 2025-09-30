#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mount.h>

void init_entropy_pool();

int main(int argc, char** argv, char** envp) {

    if (!mount("none", "/proc", "proc", 0, NULL)) {
        printf("succeeded to mount!");
    } else {
        perror("mount");
        exit(EXIT_FAILURE);
    }

    init_entropy_pool();

    char *newargv[] = { "/opt/java/openjdk/bin/java",
                            "-Dfile.encoding=UTF-8",
                            "-Xshareclasses:cacheDir=/javaSharedCache,readonly",
                            "-Xquickstart",
                            "-jar", "/javaAction/build/libs/javaAction-all.jar", NULL };
    char *newenvp[] = { "LANG=en_US.UTF-8", 
                        "LANGUAGE=en_US:en",
                        "LC_ALL=en_US.UTF-8",
                        "JAVA_VERSION=jdk8u362-b09_openj9-0.36.0",
                        "JAVA_HOME=/opt/java/openjdk",
                        "JAVA_TOOL_OPTIONS=-XX:+IgnoreUnrecognizedVMOptions -XX:+PortableSharedCache -XX:+IdleTuningGcOnIdle -Xshareclasses:name=openj9_system_scc,cacheDir=/opt/java/.scc,readonly,nonFatal",
                        "VERSION=8",
                        "UPDATE=222",
                        "BUILD=10", NULL };
    execve("/opt/java/openjdk/bin/java", newargv, newenvp);
    perror("execve");
    exit(EXIT_FAILURE);
}
