#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "com_jni_TLS.h"

#define PCG_STATE_SETSEQ_64_INITIALIZER                                        \
    { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }

typedef struct random_t {
    unsigned long state;
    unsigned long inc;
} random_t;

random_t vstate = PCG_STATE_SETSEQ_64_INITIALIZER;

uint64_t igraph_rng_pcg32_get(random_t *rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc | 1);
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

JNIEXPORT long JNICALL Java_com_jni_TLS_tls(JNIEnv *env, jobject obj) {
    struct timespec tstart={0,0}, tend={0,0};
    clock_gettime(CLOCK_MONOTONIC_RAW, &tstart);

    for (long i = 0; i < 0x100000; i++) {
        igraph_rng_pcg32_get(&vstate);
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &tend);
    long ns = ((long)tend.tv_sec * 1.0e9 + tend.tv_nsec) - ((long)tstart.tv_sec * 1.0e9 + tstart.tv_nsec);
    return ns;
}
