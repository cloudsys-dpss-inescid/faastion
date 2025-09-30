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

    char *newargv[] = { "/bin/proxy", NULL };
    char *newenvp[] = { "PATH=/usr/local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 
	                "LANG=en_US.UTF-8",
			"GPG_KEY=E3FF2839C048B25C084DEBE9B26995E310250568",
                        "LANGUAGE=en_US:en",
			"PYTHON_VERSION=3.9.6",
                        "PYTHON_PIP_VERSION=21.2.3",
                        "PYTHON_GET_PIP_URL=https://github.com/pypa/get-pip/raw/c20b0cfd643cd4a19246ccf204e2997af70f6b21/public/get-pip.py",
                        "PYTHON_GET_PIP_SHA256=fa6f3fb93cce234cd4e8dd2beb54a51ab9c247653b52855a48dd44e6b21ff28b",
                        "OW_LOG_INIT_ERROR=1",
                        "OW_WAIT_FOR_ACK=1",
                        "OW_EXECUTION_ENV=openwhisk/action-python-v3.9",
                        "OW_COMPILER=/bin/compile", NULL };
    execve("/bin/proxy", newargv, newenvp);
    perror("execve");
    exit(EXIT_FAILURE);
}
