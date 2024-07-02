package com.videoprocessing;

import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.net.URL;
import java.net.URLConnection;
import net.bramp.ffmpeg.FFmpeg;
import net.bramp.ffmpeg.FFmpegExecutor;
import net.bramp.ffmpeg.builder.FFmpegBuilder;

import com.google.gson.JsonObject;

public class VideoProcessing {

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

    private static void ffmpeg(String fileName) throws Exception{
        FFmpegBuilder builder = new FFmpegBuilder()
          .setInput(fileName) // Filename, or a FFmpegProbeResult
          .overrideOutputFiles(true) // Override the output if it exists
          .addOutput("out"+fileName) // Filename for the destination
          .setFormat("mp4") // Format is inferred from filename, or can be set
          .setVideoResolution(640, 480) // at 640x480 resolution
          .setStrict(FFmpegBuilder.Strict.EXPERIMENTAL) // Allow FFmpeg to use experimental specs
          .done();
        new FFmpegExecutor(new FFmpeg("/tmp/ffmpeg")).createJob(builder).run();
    }

    public static JsonObject main(JsonObject args) {
        JsonObject response = new JsonObject();

        if (!new File("/tmp/ffmpeg").exists()) {
            File file = new File("/tmp/ffmpeg");
            try (FileOutputStream stream = new FileOutputStream(file)) {
                stream.write(downloadBytes(args.getAsJsonPrimitive("ffmpeg_url").getAsString()));
                file.setWritable(false);
                file.setReadable(true);
                file.setExecutable(true);
            } catch (Exception e) {
            	response.addProperty("output", e.getMessage());
                 e.printStackTrace();
             }
        }

        try (FileOutputStream stream = new FileOutputStream("video.mp4")) {
            stream.write(downloadBytes(args.getAsJsonPrimitive("video_url").getAsString()));
        } catch (Exception e) {
        	response.addProperty("output", e.getMessage());
             e.printStackTrace();
         }
        
        try {
            ffmpeg("video.mp4");
        } catch (Exception e) {
        	response.addProperty("output", e.getMessage());
            e.printStackTrace();
        }

        response.addProperty("video", "video.mp4");
        return response;
    }

    public static void main(String[] args) {
        JsonObject response = new JsonObject();
        response.addProperty("ffmpeg_url", "http://127.0.0.1:8000/ffmpeg");
        response.addProperty("video_url", "http://127.0.0.1:8000/file_example_MP4_480_1_5MG.mp4");
        System.out.println(main(response));
    }
}
