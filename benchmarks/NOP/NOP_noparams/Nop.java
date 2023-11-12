public class Nop {

    static {
        System.loadLibrary("Nop");
    }
    
    private native void voidRun();
    
    public static void main(String[] args) {
	    long start = System.nanoTime();
    	    new Nop().voidRun();
	    long end = System.nanoTime();
	    double elapsedTime = ((double)end - start) / 1000000;
            System.out.println(elapsedTime);
    }

}
