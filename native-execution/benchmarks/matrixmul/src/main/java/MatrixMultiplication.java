public class MatrixMultiplication{
    static {
        System.loadLibrary("matrix");
    }
    
    private native void multiply();

    public static void main(String[] args) {
        new MatrixMultiplication().multiply();
    }

}
