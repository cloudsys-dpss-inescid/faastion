#include <stdio.h>
#include <stdlib.h>
#include "../graal_isolate.h"
#include <string.h>
#include <stddef.h>

void* buff = NULL;

int graal_create_isolate (graal_create_isolate_params_t* params, graal_isolate_t** isolate, graal_isolatethread_t** thread) {
    *thread = NULL;
    *isolate = NULL;
    return 0;
}

int graal_tear_down_isolate(graal_isolatethread_t* thread) {
    return 0;
}

void entrypoint(graal_isolatethread_t* thread, const char* fin, const char* fout, unsigned long fout_len) {

    if (buff != NULL) {
        printf("buff is NOT NULL\n");
        // see if restore worked
        printf("It's contents are: %s\n", (char *) buff);

        void* temp = malloc(300);
        ptrdiff_t dif = (char*) temp - (char*) buff;
        printf("Difference between pointers: %td\n", dif);
        printf("freeing buff = %p\n", buff);
        free(buff);

        buff = malloc(128);
        strcpy(buff, "new str");
        // see if buff keeps old addr 0xa00000001500
        printf("new buff addr = %p\n", buff);
        free(buff);
    }

    printf("start of default application!!!\n\n");
    void* padding = malloc(440);
    printf("Added padding @ %p\n", padding);

    buff = malloc(512 * sizeof(char));
    char* test_str = "this should be copied";
    strcpy(buff, test_str);
    printf("Stored at %p: %s\n", buff, (char *) buff);

    void* last = calloc(512, 1);
    printf("last @ %p\n", last);
}

int graal_detach_thread(graal_isolatethread_t* thread) {
    return 0;
}

int graal_attach_thread(graal_isolate_t* isolate, graal_isolatethread_t** thread) {
    *thread = NULL;
    return 0;
}
