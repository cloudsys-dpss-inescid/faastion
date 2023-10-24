public class Nop {

    static {
        System.loadLibrary("Nop");
    }
    
    private native void voidRun(int a, int b);
    
    public static void main(String[] args) {
            long start = System.nanoTime();
	    new Nop().voidRun(1,2);
            long end = System.nanoTime();
	    double elapsedTime = ((double)end - start) / 1000000; 
	    System.out.println(elapsedTime);
    }

}
