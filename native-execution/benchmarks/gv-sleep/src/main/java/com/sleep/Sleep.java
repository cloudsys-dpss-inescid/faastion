package com.sleep;

import java.lang.InterruptedException;
import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class Sleep {

    static Map<String, Object> output = new HashMap<>();

    static byte[] buffer;

    public static void memory(int bytes) {
        buffer = new byte[bytes];
        for (int i = 0; i < bytes; i++) {
            buffer[i] = 1;
        }
    }

    public static void sleep(long millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException ie) {
            output.put("Log", "InterruptedException");
        }
    }

    public static Map<String, Object> main(Map<String, Object> input) {
        memory(Integer.parseInt((String)input.get("memory")));
        sleep(Long.parseLong((String)input.get("sleep")));
        return output;
    }

}
