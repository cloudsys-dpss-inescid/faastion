package com.genericapp;

import java.security.MessageDigest;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;

public class GenericApp {

    static Map<String, Object> output = new HashMap<>();

    static byte[] buffer;

    public static String genericStuff(int bytes, int duration) throws Exception {
        buffer = new byte[bytes];
        long start = System.currentTimeMillis();
        Random rand = new Random();
        String result = "";

        while (System.currentTimeMillis() < (start + duration)) {
            // This simulates going over the network to getch some data.
            Thread.sleep(100);
            rand.nextBytes(buffer);
            result = new String(MessageDigest.getInstance("MD5").digest(buffer));
        }
        return result;
    }

    public static Map<String, Object> main(Map<String, Object> input) throws Exception {
        genericStuff(4000000, 1000);
        return output;
    }

    public static void main(String[] args) throws Exception {
        Map<String, Object> input = new HashMap<>();
        main(input);
    }
}