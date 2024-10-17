import java.util.Random;

public class MatrixMultiplication{

	static{
			System.loadLibrary("matmul");
	}

    public native int[][] multiplyMatrices(int[][] matrixA, int[][] matrixB);

	public static void main(String[] args){
		MatrixMultiplication app = new MatrixMultiplication();
        Random random = new Random();

        // Define matrix dimensions
        int rowsA = 3;
        int colsA = 2;  // For example, A is 3x2
        int rowsB = 2; 
        int colsB = 4;  // For example, B is 2x4

        // Generate random matrices
        int[][] matrixA = new int[rowsA][colsA];
        int[][] matrixB = new int[rowsB][colsB];

        // Fill matrixA with random integers
        for (int i = 0; i < rowsA; i++) {
            for (int j = 0; j < colsA; j++) {
                matrixA[i][j] = random.nextInt(10);  // Random values from 0 to 9
            }
        }

        // Fill matrixB with random integers
        for (int i = 0; i < rowsB; i++) {
            for (int j = 0; j < colsB; j++) {
                matrixB[i][j] = random.nextInt(10);  // Random values from 0 to 9
            }
        }

        // Print the input matrices
        System.out.println("Matrix A:");
        printMatrix(matrixA);
        System.out.println("Matrix B:");
        printMatrix(matrixB);

        // Call the native method to multiply the matrices
        int[][] result = app.multiplyMatrices(matrixA, matrixB);

        // Print the resulting matrix
        System.out.println("Resulting Matrix:");
        printMatrix(result);
    }

    // Method to print matrices
    private static void printMatrix(int[][] matrix) {
        for (int[] row : matrix) {
            for (int value : row) {
                System.out.print(value + " ");
            }
            System.out.println();
        }
    }
}
