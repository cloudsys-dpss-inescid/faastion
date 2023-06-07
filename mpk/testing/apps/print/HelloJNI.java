public class HelloJNI {
    static {
        System.loadLibrary("hello");
    }

    public static native void print(String message);

    public static void main(String[] args) {
        String msg= "Hello, World!";
        print(msg);
    }
}
