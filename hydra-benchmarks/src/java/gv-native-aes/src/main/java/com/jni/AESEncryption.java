package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public class AESEncryption {
    static {
        System.loadLibrary("aes-jni");
    }

    public static native void cipher();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        cipher();
        return output;
    }


    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
