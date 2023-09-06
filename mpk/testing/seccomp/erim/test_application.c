#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main() {
    size_t size = sizeof(int);

    void *mapped_mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, -1, 0);
    if (mapped_mem == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    return 0;
}
