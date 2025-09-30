package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;


public class BFS {
    static {
        System.loadLibrary("bfs-jni");
    }

    public static native void bfs();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
	bfs();
        return output;
    }


    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
