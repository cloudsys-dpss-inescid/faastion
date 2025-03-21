package com.jni;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.File;
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

public class FileHashing {
    static {
        System.loadLibrary("filehashing-jni");
    }
    
    public static native void hash();

    public static boolean downloadFile(String url, String filePath) {
        InputStream is = null;
        FileOutputStream fos = null;
        FileOutputStream ignore = null;
        try {
            boolean fileExists = (new File(filePath)).exists();
            URLConnection conn = new URL(url).openConnection();
            is = conn.getInputStream();

            if (!fileExists) {
                fos = new FileOutputStream(filePath);
            }

            ignore = new FileOutputStream("/dev/null");

            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1) {
                if (fileExists) {
                    ignore.write(buffer, 0, bytesRead);
                } else {
                    fos.write(buffer, 0, bytesRead);
                }
            }
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (is != null) is.close();
                if (fos != null) fos.close();
                if (ignore != null) ignore.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public static void deleteFile(String filePath) {
        try {
            Path path = Paths.get(filePath);
            Files.deleteIfExists(path);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static HashMap<String, Object> main(Map<String, Object> args) {
	    HashMap<String, Object> output = new HashMap<>();

        String url = "http://127.0.0.1:8000/snap.png";
        String filePath = "/tmp/snap.png";
        boolean success = downloadFile(url, filePath);

        if (success) {
            hash();
        }
        output.put("success", success);

        return output;
    }

    public static void main(String[] args) {
    	HashMap<String, Object> output = new HashMap<>();
    	output = main(output);
    }
}
