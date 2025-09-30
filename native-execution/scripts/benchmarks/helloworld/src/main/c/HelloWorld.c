#include "../../../build/generated/sources/headers/java/main/HelloWorld.h"
#include <stdio.h>

JNIEXPORT void JNICALL Java_HelloWorld_helloworld(JNIEnv *env, jobject obj) {
    printf("Hello World!");
}
