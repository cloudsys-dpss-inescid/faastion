package com.matrix_mul;

import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class MatrixMul {

    private static final int ROWS = 3;
    private static final int COLS = 3;


    public static int[][] allocateMatrix() {
        return new int[ROWS][COLS];
    }

    public static void matrixGenerator(int[][] matrix) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                matrix[i][j] = 10000;
            }
        }
    }

    public static void multiplyMatrices(int[][] matrix1, int[][] matrix2, int[][] result) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                result[i][j] = 0;
                for (int k = 0; k < ROWS; k++) {
                    result[i][j] += matrix1[i][k] * matrix2[k][j];
                }
            }
        }
    }

    public static void printMatrix(int[][] matrix) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                System.out.print(matrix[i][j] + " ");
            }
            System.out.println();
        }
    }

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        int[][] matrix1 = allocateMatrix();
        int[][] matrix2 = allocateMatrix();
        int[][] result = allocateMatrix();
        
        matrixGenerator(matrix1);
        matrixGenerator(matrix2);
        
        multiplyMatrices(matrix1, matrix2, result);
        
        printMatrix(result);
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }
}
