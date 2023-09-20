public class HelloJNI {
    static {
        System.loadLibrary("hello");
    }

    public static native void print();

    public static void main(String[] args) {
        print();
    }
}
