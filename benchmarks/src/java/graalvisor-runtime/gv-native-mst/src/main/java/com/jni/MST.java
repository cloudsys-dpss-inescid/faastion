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
        HashMap<String, Object> output;
        long tstart, tend;        

        int iter = args.length > 0 ? Integer.parseInt(args[0]) : 1;
        for (int i = 0; i < iter; i++) {
            output = new HashMap<>();
            tstart = System.nanoTime();
            output = main(output);
            tend = System.nanoTime();
            System.out.println("Total execution time (us): " + ((tend - tstart) / 1000));
        }
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
