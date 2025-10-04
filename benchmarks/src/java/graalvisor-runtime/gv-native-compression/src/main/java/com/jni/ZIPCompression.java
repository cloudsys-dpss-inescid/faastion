package com.jni;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.math.BigInteger;
import java.util.Map;
import java.util.Random;
import java.util.HashMap;

import java.util.concurrent.ThreadLocalRandom;

import org.graalvm.word.UnsignedWord;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

import com.fasterxml.jackson.jr.ob.JSON;

public class ZIPCompression {
    static {
        System.loadLibrary("zip-jni");
    }

    public static String IMG_FILENAME = String.format("img-%d.png", ThreadLocalRandom.current().nextInt(0, 1024 + 1));
    
    public static native void compress(String filePath);

    public static boolean downloadFile(String url, String filePath) {
        InputStream is = null;
        FileOutputStream fos = null;
        try {
            URLConnection conn = new URL(url).openConnection();
            is = conn.getInputStream();
            fos = new FileOutputStream(filePath);

            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1) {
                fos.write(buffer, 0, bytesRead);
            }
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (is != null) is.close();
                if (fos != null) fos.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static HashMap<String, Object> main(Map<String, Object> input) {
        String tmpDir = (String) input.get("tmpDir");
        String filePath = tmpDir + "/" + IMG_FILENAME;

        HashMap<String, Object> output = new HashMap<>();

        String url = "http://127.0.0.1:8000/snap.png";
        boolean success = downloadFile(url, filePath);

        if (success) {
            compress(filePath);
        }
        output.put("success", success);
        
        return output;
    }

    public static void main(String[] args) {
    	HashMap<String, Object> output = new HashMap<>();
    	output = main(output);
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
