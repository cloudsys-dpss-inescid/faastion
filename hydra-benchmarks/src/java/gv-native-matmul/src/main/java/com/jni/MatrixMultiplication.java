package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public class MatrixMultiplication {

    static {
        System.loadLibrary("matmul-jni");
    }
    
    public static native void multiply();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        multiply();
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }

}
