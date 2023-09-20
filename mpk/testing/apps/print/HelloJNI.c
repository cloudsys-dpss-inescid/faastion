#include <stdio.h>
#include "HelloJNI.h"

JNIEXPORT void JNICALL Java_HelloJNI_print(JNIEnv *env, jobject obj) {
    fprintf(stderr, "Hello World!\n");
    return;
}
