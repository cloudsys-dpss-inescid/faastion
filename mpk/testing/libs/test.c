#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

int main() {
    // Load the shared library
    void *handle = dlopen("./libhello.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        exit(1);
    }

    dlclose(handle);

    return 0;
}
