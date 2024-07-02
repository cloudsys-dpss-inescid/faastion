package com.dynamichtml;

import java.util.Map;

import com.oracle.svm.graalvisor.utils.JsonUtils;
import com.oracle.svm.graalvisor.polyglot.PolyglotHostAccess;
import com.oracle.svm.graalvisor.polyglot.PolyglotEngine;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.HostAccess;

import com.github.mustachejava.DefaultMustacheFactory;

import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.HashMap;

public class DynamicHTML extends PolyglotHostAccess {

    private static DynamicHTMLEngine engine;
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

    // Note: Replacement for nodejs mustache package.
    @HostAccess.Export
    public String mustache(String template, String arguments) {
        Map<String, Object> scopes = JsonUtils.jsonToMap(arguments);
        try {
            Writer writer = new StringWriter();
            new DefaultMustacheFactory().compile(new StringReader(template), "template").execute(writer, scopes);
            writer.flush();
            return writer.toString();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }


    static class DynamicHTMLEngine extends PolyglotEngine {

        public void addBindings(String language, Context context) {
            context.getBindings(language).putMember("polyHostAccess", new DynamicHTML());
        }
    }

    private static DynamicHTMLEngine getEngine() {
        if (engine == null) {
            engine = new DynamicHTMLEngine();
        }
        return engine;
    }

    public static HashMap<String, Object> main(Map<String, Object> args) {
        HashMap<String, Object> output = new HashMap<>();
        String url = (String) args.get("url");
        String username = (String) args.get("username");
        String nsize = (String) args.get("nsize");
        DynamicHTMLEngine engine = getEngine();
        output.put("output", engine.invoke(language, source, entrypoint, String.format("%s;%s;%s", url, username, nsize)));
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output.put("url", "http://127.0.0.1:8000/template.html");
        output.put("username", "rbruno");
        output.put("nsize", "10");
        output = main(output);
        System.out.println(output);
    }
}
