package com.sleep;

import java.lang.InterruptedException;
import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class Sleep {

    static Map<String, Object> output = new HashMap<>();

    public static void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException ie) {
            output.put("Log", "InterruptedException");
        }
    }

    public static Map<String, Object> main(Map<String, Object> input) {
        sleep(50);
        return output;
    }

    public static void main(String[] args) {
        Map<String, Object> output = new HashMap<>();
        output = main(output);
    }

}
