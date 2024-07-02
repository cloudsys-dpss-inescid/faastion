package com.videoprocessing;

import java.util.Map;

import com.oracle.svm.graalvisor.polyglot.PolyglotEngine;
import com.oracle.svm.graalvisor.polyglot.PolyglotHostAccess;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;

public class VideoProcessing extends PolyglotHostAccess {

    private static PolyglotEngine engine;
    private static String language;
    private static String source;
    private static String entrypoint;

    static {
        try {
            language = System.getProperty("com.oracle.svm.graalvisor.polyglotengine.language");
            source = Files.readString(Paths.get(System.getProperty("com.oracle.svm.graalvisor.polyglotengine.source")));
            entrypoint = System.getProperty("com.oracle.svm.graalvisor.polyglotengine.entrypoint");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static PolyglotEngine getEngine() {
        if (engine == null) {
            engine = new PolyglotEngine();
        }
        return engine;
    }

    public static HashMap<String, Object> main(Map<String, Object> args) {
        HashMap<String, Object> output = new HashMap<>();
        PolyglotEngine engine = getEngine();
        String video = (String) args.get("video");
        String ffmpeg = (String) args.get("ffmpeg");
        output.put("output", engine.invoke(language, source, entrypoint, String.format("%s;%s", ffmpeg, video)));
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output.put("video", "http://127.0.0.1:8000/video.mp4");
        output.put("ffmpeg", "http://127.0.0.1:8000/ffmpeg");
        output = main(output);
        System.out.println(output);
    }
}
