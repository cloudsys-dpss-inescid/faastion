package com.videoprocessing;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Map;

import net.bramp.ffmpeg.FFmpeg;
import net.bramp.ffmpeg.FFmpegExecutor;
import net.bramp.ffmpeg.builder.FFmpegBuilder;

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

    private static void ffmpeg(String fileName) throws Exception{
        FFmpegBuilder builder = new FFmpegBuilder()
          .setInput(fileName) // Filename, or a FFmpegProbeResult
          .overrideOutputFiles(true) // Override the output if it exists
          .addOutput("out"+fileName) // Filename for the destination
          .setFormat("mp4") // Format is inferred from filename, or can be set
          .setVideoResolution(640, 480) // at 640x480 resolution
          .setStrict(FFmpegBuilder.Strict.EXPERIMENTAL) // Allow FFmpeg to use experimental specs
          .done();
        new FFmpegExecutor(new FFmpeg("./ffmpeg")).createJob(builder).run();
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
        
        try (FileOutputStream stream = new FileOutputStream("video.mp4")) {
            stream.write(downloadBytes(video_url));
        } catch (Exception e) {
             output.put("output", e.getMessage());
             e.printStackTrace();
         }
        
        try {
            ffmpeg("video.mp4");
        } catch (Exception e) {
            output.put("output", e.getMessage());
            e.printStackTrace();
        }
        
        output.put("output", "video.mp4");
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }
}
