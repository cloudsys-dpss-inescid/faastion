package com.jni;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import org.graalvm.word.UnsignedWord;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

import com.fasterxml.jackson.jr.ob.JSON;

public class TLS {
    static {
        System.loadLibrary("tls-jni");
    }

    public static native long tls();

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        long start = System.nanoTime();
	    long native_time = tls();
        output.put("java time (us)", String.valueOf((System.nanoTime() - start) / 1000));
        output.put("native time (us)", String.valueOf(native_time / 1000));
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }

    public static Map<String, Object> jsonToMap(String jsonString) {
        try {
            if (jsonString != null && !jsonString.isEmpty()) {
                return JSON.std.mapFrom(jsonString);
            }
        } catch (IOException e) {
            e.printStackTrace(System.err);
        }
        return new HashMap<>();
    }

    /* For c-API invocations. */
    @CEntryPoint(name = "entrypoint")
    public static void main(IsolateThread thread, CCharPointer fin, CCharPointer fout, UnsignedWord foutLen) {
        String input = CTypeConversion.toJavaString(fin);
        Map<String, Object> map = jsonToMap(input);
        String output = main(map).toString();

        int len = Math.min((int) foutLen.rawValue() - 1, output.length());
        if (len > 0) {
            CTypeConversion.toCString(output.substring(0, len), fout, foutLen);
        }
    }
}
