package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public class MST {
    static {
        System.loadLibrary("mst-jni");
    }

    public static native void mst();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
	mst();
        return output;
    }


    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
