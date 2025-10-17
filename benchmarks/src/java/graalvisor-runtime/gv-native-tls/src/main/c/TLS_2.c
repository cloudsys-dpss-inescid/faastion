#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "com_jni_TLS.h"

static long global = 0;

JNIEXPORT long JNICALL Java_com_jni_TLS_tls(JNIEnv *env, jobject obj) {
    struct timespec tstart={0,0}, tend={0,0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);

    for (long i = 0; i < 0x500000; i++) {
        global += 1;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &tend);
    long ns = ((long)tend.tv_sec * 1.0e9 + tend.tv_nsec) - ((long)tstart.tv_sec * 1.0e9 + tstart.tv_nsec);
    return ns;
}
