#include "../../../build/generated/sources/headers/java/main/MatrixMultiplication.h"
#include <stdio.h>
#include <stdlib.h>

JNIEXPORT jobjectArray JNICALL Java_MatrixMultiplication_multiplyMatrices(JNIEnv *env, jobject obj, jobjectArray matrixA, jobjectArray matrixB) {
    // Get the dimensions of the matrices
    jint rowsA = (*env)->GetArrayLength(env, matrixA);
    jint colsA = (*env)->GetArrayLength(env, (jobjectArray) (*env)->GetObjectArrayElement(env, matrixA, 0));
    jint rowsB = (*env)->GetArrayLength(env, matrixB);
    jint colsB = (*env)->GetArrayLength(env, (jobjectArray) (*env)->GetObjectArrayElement(env, matrixB, 0));

    // Check for valid matrix dimensions
    if (colsA != rowsB) {
        return NULL;  // Return null if matrices cannot be multiplied
    }

    // Create the resulting matrix
    jobjectArray result = (*env)->NewObjectArray(env, rowsA, (*env)->GetObjectClass(env, (*env)->GetObjectArrayElement(env, matrixB, 0)), NULL);

    for (jint i = 0; i < rowsA; i++) {
        jintArray row = (*env)->NewIntArray(env, colsB);
        for (jint j = 0; j < colsB; j++) {
            jint sum = 0;
            for (jint k = 0; k < colsA; k++) {
                jint valueA = (*env)->GetIntArrayElements(env, (jintArray)(*env)->GetObjectArrayElement(env, matrixA, i), NULL)[k];
                jint valueB = (*env)->GetIntArrayElements(env, (jintArray)(*env)->GetObjectArrayElement(env, matrixB, k), NULL)[j];
                sum += valueA * valueB;
            }
            (*env)->SetIntArrayRegion(env, row, j, 1, &sum);
        }
        (*env)->SetObjectArrayElement(env, result, i, row);
    }

    return result;  // Return the resulting matrix
}