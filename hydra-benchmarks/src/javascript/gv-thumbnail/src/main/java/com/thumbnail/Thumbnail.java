package com.thumbnail;

import java.util.Map;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.HostAccess;

import com.oracle.svm.graalvisor.polyglot.PolyglotEngine;
import com.oracle.svm.graalvisor.polyglot.PolyglotHostAccess;

import com.criteo.vips.VipsImage;
import com.criteo.vips.enums.VipsImageFormat;

import java.awt.Dimension;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;

public class Thumbnail extends PolyglotHostAccess {

    private static ThumbnailEngine engine;
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

    @HostAccess.Export
    public byte[] resize(byte[] bytes, float ratio) {
        try {
            VipsImage image = new VipsImage(bytes, bytes.length);
            int width = (int) (image.getWidth() * ratio);
            int height =(int) (image.getHeight() * ratio);
            image.thumbnailImage(new Dimension(width, height), true);
            bytes = image.writeToArray(VipsImageFormat.PNG, false);
            image.release();
            return bytes;
        } catch (Throwable e) {
            e.printStackTrace();
            return null;
        }
    }

    static class ThumbnailEngine extends PolyglotEngine {

        public void addBindings(String language, Context context) {
            context.getBindings(language).putMember("polyHostAccess", new Thumbnail());
        }
    }

    private static ThumbnailEngine getEngine() {
        if (engine == null) {
            engine = new ThumbnailEngine();
        }
        return engine;
    }

    public static HashMap<String, Object> main(Map<String, Object> args) {
        HashMap<String, Object> output = new HashMap<>();
        ThumbnailEngine engine = getEngine();
        output.put("output", engine.invoke(language, source, entrypoint, (String) args.get("url")));
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output.put("url", "http://127.0.0.1:8000/snap.png");
        output = main(output);
        System.out.println(output);
    }
}
