package com.thumbnail;

import java.io.IOException;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.ByteArrayOutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Map;
import java.util.HashMap;

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.imgproc.Imgproc;
import org.opencv.core.Core;
import org.opencv.core.Size;
import nu.pattern.OpenCV;

import java.util.concurrent.ThreadLocalRandom;

public class Thumbnail {
    
    public static String IMG_FILENAME = String.format("img-%d.png", ThreadLocalRandom.current().nextInt(0, 1024 + 1));

    public static boolean resize(String filePath, double ratio, String outFile) {
        nu.pattern.OpenCV.loadLocally();
		Mat image = Imgcodecs.imread(filePath);
		Imgproc.resize(image, image, new Size(0, 0), ratio, ratio);
		Imgcodecs.imwrite(outFile, image);
        return true;
    }

    public static byte[] fromInputStream(InputStream is) throws Exception {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        int nRead;
        byte[] data = new byte[16384];

        while ((nRead = is.read(data, 0, data.length)) != -1) {
            buffer.write(data, 0, nRead);
        }

        return buffer.toByteArray();
    }

    public static byte[] downloadBytes(String url) {
        try {
            URLConnection conn = new URL(url).openConnection();
            InputStream is = conn.getInputStream();
            byte[] bytes = fromInputStream(is); 
            is.close();
            return bytes;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static HashMap<String, Object> main(Map<String, Object> input) {
        String tmpDir = (String) input.get("tmpDir");
        String url = "http://127.0.0.1:8000/snap.png";
        String filePath = tmpDir + "/" + IMG_FILENAME;
        String outFile = tmpDir + "/output.png"; 
        HashMap<String, Object> output = new HashMap<>();
        boolean success = false;
        try {
            FileOutputStream stream = new FileOutputStream(filePath);
            stream.write(downloadBytes(url));
            success = resize(filePath, 0.25, outFile);
        } catch (Exception e) {}
        output.put("success", success);
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output.put("tmpDir", "/tmp/sandbox-0");
        output = main(output);
        System.out.println(output);
    }
}
