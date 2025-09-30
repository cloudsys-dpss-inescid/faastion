package com.videoprocessing;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Map;

import java.util.concurrent.TimeUnit;

import java.util.HashMap;

public class VideoProcessing {
    
    private static final String ffmpeg_url = "http://127.0.0.1:8000/ffmpeg";
    private static final String video_url = "http://127.0.0.1:8000/video.mp4";

    public static byte[] downloadBytes(String url) {
        try {
            URLConnection conn = new URL(url).openConnection();
            InputStream is = conn.getInputStream();
            byte[] bytes = is.readAllBytes();
            is.close();
            return bytes;
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public static HashMap<String, Object> main(Map<String, Object> args) {
        HashMap<String, Object> output = new HashMap<>();
        
        if (!new File("ffmpeg").exists()) {
            File file = new File("ffmpeg");
            try (FileOutputStream stream = new FileOutputStream(file)) {
                stream.write(downloadBytes(ffmpeg_url));
                file.setWritable(false);
                file.setReadable(true);
                file.setExecutable(true);
            } catch (Exception e) {
                 output.put("output", e.getMessage());
                 e.printStackTrace();
             } 
        }
        
        byte[] bytes = downloadBytes(video_url);
        if (!new File("video.mp4").exists()) {
            try (FileOutputStream stream = new FileOutputStream("video.mp4")) {
                stream.write(bytes);
            } catch (Exception e) {
                output.put("output", e.getMessage());
                e.printStackTrace();
            }
        }
        
        try {
            ProcessBuilder pb = new ProcessBuilder(
                "./ffmpeg",
                "-nostdin",
                "-y",
                "-i", "video.mp4",
                "-s", "640x480",
                "-c:a", "copy",
                "outvideo.mp4"
            );
            pb.redirectErrorStream(true);
            Process p = pb.start();
            p.waitFor();
            output.put("output", String.valueOf(p.exitValue()));
        } catch (Exception e) {
            output.put("output", e.getMessage());
            e.printStackTrace();
        }
        
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }
}
