#include <stdio.h>
#include <stdlib.h>
#include "seccomp.h"

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s mode command [args]\n", argv[0]);
        exit(1);
    }

    exec(argv+1);

    return 0;
}