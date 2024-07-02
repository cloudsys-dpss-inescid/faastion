#include "com_jni_Factorization.h"
#include <stdio.h>
#include <stdlib.h>
#define MAX_FACTORS 500


JNIEXPORT void JNICALL Java_com_jni_Factorization_computeFactors(JNIEnv *env, jobject object) {
    int* num_factors = (int*)malloc(sizeof(int)*MAX_FACTORS);

    int count = 0;
    int number = 10000000;
    
    for (int i = 1; i <= number; i++) {
        if (count == MAX_FACTORS) {
            break;
        }
        if((number % i) == 0){
            num_factors[count++] = i;
        }
    }
}
