#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv) {
    sleep(1);

    FILE* f = fopen("helloworld.log", "w");
    fprintf(f, "Hello from fprintf!\n");
    fclose(f);

    printf("Hello from printf!\n");
    return 0;
}