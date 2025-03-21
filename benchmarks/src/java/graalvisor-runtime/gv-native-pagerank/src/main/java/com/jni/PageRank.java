package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public class PageRank {
    static {
        System.loadLibrary("pagerank-jni");
    }

    public static native void pagerank();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
	pagerank();
        return output;
    }


    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
