public class matrix_multiplication{
    static {
        System.loadLibrary("native");
    }
    
    private native void multiply();

    public static void main(String[] args) {
        new matrix_multiplication().multiply();
    }

}
