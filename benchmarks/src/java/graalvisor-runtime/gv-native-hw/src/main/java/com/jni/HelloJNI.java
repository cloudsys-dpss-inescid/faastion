package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public class HelloJNI {
    static {
        System.loadLibrary("nativehw-jni");
    }

    public static native void printHello();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();

        Thread t1 = new Thread(() -> {
            printHello();
        });
        Thread t2 = new Thread(() -> {
            printHello();
        });
        t1.start();
        t2.start();
        
        try {
            t1.join();
            t2.join();
        } catch (InterruptedException ie) {
            ie.printStackTrace();
        }

        return output;
    }


    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
