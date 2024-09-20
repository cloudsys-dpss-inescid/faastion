#include <stdio.h>
#include <unistd.h>
#include "com_jni_HelloJNI.h"

JNIEXPORT void JNICALL Java_com_jni_HelloJNI_printHello(JNIEnv *env, jobject obj, jint age) {
    fprintf(stderr, "Hello, I am %d years old.\n", age);
    // const jchar *strp = (*env)->GetStringChars(env, name, NULL);
    // fprintf(stderr, "name: %s, age: %d\n", (char *)strp, age);
    return;
}
