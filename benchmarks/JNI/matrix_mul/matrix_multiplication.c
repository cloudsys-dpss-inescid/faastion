#include "matrix_multiplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROWS 3
#define COLS 3

void matrixGenerator(int matrix[ROWS][COLS]){
	srand(time(NULL));
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            matrix[i][j] = rand() % 1000; 
        }
    }
}

JNIEXPORT void JNICALL Java_matrix_1multiplication_multiply
  (JNIEnv *, jobject){

	int matrix1[ROWS][COLS];
	int matrix2[ROWS][COLS];
	
	matrixGenerator(matrix1);
	matrixGenerator(matrix2);

    int result[ROWS][COLS];

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            result[i][j] = 0;
            for (int k = 0; k < ROWS; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }
	
	
	
}
