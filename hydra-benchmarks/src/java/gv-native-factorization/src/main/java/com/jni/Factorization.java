package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public class Factorization {

    static {
        System.loadLibrary("factors-jni");
    }
    
    public static native void computeFactors();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        computeFactors();
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }

}
