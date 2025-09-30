#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "com_jni_HelloJNI.h"

static __thread int var = 1;

JNIEXPORT void JNICALL Java_com_jni_HelloJNI_printHello(JNIEnv *env, jobject obj) {
    fprintf(stderr, "Hello World!\n");
    fprintf(stderr, "&errno: %p\n", &errno);
    fprintf(stderr, "errno: %d\n", errno);
    int *addr = malloc(sizeof(int));
    fprintf(stderr, "&var: %p\n", &var);
    fprintf(stderr, "var: %d\n", var++);
    fprintf(stderr, "var: %d\n", var);

    char *libpath = secure_getenv("LD_LIBRARY_PATH");
    if (libpath != NULL)
        printf("%s\n", libpath);

    return;
}
