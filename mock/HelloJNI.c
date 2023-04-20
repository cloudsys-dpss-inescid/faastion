#include <stdio.h>
#include <jni.h>
#include "HelloJNI.h"

JNIEXPORT void JNICALL Java_HelloJNI_print(JNIEnv *env, jobject obj, jstring message) {
    const char *c_message = (*env)->GetStringUTFChars(env, message, NULL);
    printf("%s\n", c_message);
    (*env)->ReleaseStringUTFChars(env, message, c_message);
    return;
}
