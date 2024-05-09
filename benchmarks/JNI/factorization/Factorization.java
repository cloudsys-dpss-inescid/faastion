public class Factorization {

    static {
        System.loadLibrary("factors");
    }
    
    private native void computeFactors();

    public static void main(String[] args) {
        new Factorization().computeFactors();
    }

}
