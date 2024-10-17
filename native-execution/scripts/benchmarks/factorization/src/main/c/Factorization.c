#include "../../../build/generated/sources/headers/java/main/Factorization.h"
#include <math.h>

JNIEXPORT jint JNICALL Java_Factorization_factor(JNIEnv *env, jobject obj, jint number) {
    int num = number;
    int factor_count = 0;

    // Factorize the number and count the factors
    printf("Factors of %d: ", num);

    // Handle the case for 2, the only even prime
    while (num % 2 == 0) {
        printf("%d ", 2);
        num /= 2;
        factor_count++;
    }

    // Handle odd factors from 3 upwards
    for (int i = 3; i <= sqrt(num); i += 2) {
        while (num % i == 0) {
            printf("%d ", i);
            num /= i;
            factor_count++;
        }
    }

    // If num is still greater than 2, it's prime
    if (num > 2) {
        printf("%d", num);
        factor_count++;
    }

    printf("\n");

    return factor_count;  // Return the number of factors to Java
}