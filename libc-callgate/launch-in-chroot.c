#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv) {
     if (argc != 5) {
         fprintf(stderr, "Usage: %s <chroot path> <ld library path> <ld preload path> <file-to-exec>\n", argv[0]);
         exit(-1);
     }
     chdir(argv[1]);
     if (chroot(argv[1])) {
         perror("chroot");
         exit(-1);
     }

     char *newargv[] = { NULL, NULL };
     char *newenviron[] =
     {
         argv[2],
         argv[3],
         NULL
     };
     newargv[0] = argv[4];
     execve(argv[4], newargv, newenviron);
     perror("execve");
     exit(-1);
}
