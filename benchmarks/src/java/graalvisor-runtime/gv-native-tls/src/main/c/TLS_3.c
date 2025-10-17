#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "com_jni_TLS.h"

static long global = 0;

JNIEXPORT long JNICALL Java_com_jni_TLS_tls(JNIEnv *env, jobject obj) {
    for (long i = 0; i < 0x500000; i++) {
        global += 1;
    }
    return global;
}
