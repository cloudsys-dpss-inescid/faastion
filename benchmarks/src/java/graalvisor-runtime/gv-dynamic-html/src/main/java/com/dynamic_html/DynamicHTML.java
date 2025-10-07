package com.dynamic_html;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Map;
import java.util.HashMap;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;
import java.io.FileOutputStream;
import java.io.PrintWriter;
import java.io.StringWriter;

import com.github.mustachejava.DefaultMustacheFactory;
import com.github.mustachejava.Mustache;
import com.github.mustachejava.MustacheFactory;

import java.util.List;
import java.util.Date;
import java.util.Random;
import java.text.SimpleDateFormat;
import java.util.stream.Collectors;

import org.graalvm.word.UnsignedWord;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

import com.fasterxml.jackson.jr.ob.JSON;

public class DynamicHTML {

    private static final String url = "http://127.0.0.1:8000/template.html";
    private static final int TEST_INPUT = 10;
    private static final int SMALL_INPUT = 1000;
    private static final int LARGE_INPUT = 100000;

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

    public static boolean renderTemplate(String filePath, String name, int size) {
        MustacheFactory mf = new DefaultMustacheFactory();
        Mustache mustache = mf.compile(filePath);
        Map<String, Object> contents = new HashMap();
        contents.put("username", name);
        contents.put("cur_time", new SimpleDateFormat("MM/dd/yyyy HH:mm:ss")
                        .format(new Date()));
        contents.put("random_numbers", new Random().ints(size, 0, 101)
                        .mapToObj(n -> String.valueOf(n))
                        .collect(Collectors.toList()));
        StringWriter sw = new StringWriter();
        mustache.execute(sw, contents);
        System.out.println(sw.getBuffer().substring(0,10));
        return true;
    }

    public static HashMap<String, Object> main(Map<String, Object> input) {
        String tmpDir = (String) input.get("tmpDir");
        String filePath = tmpDir + "/template.html";

        HashMap<String, Object> output = new HashMap<>();

        boolean success;
        if ((success = downloadFile(url, filePath))) {
            success = renderTemplate(filePath, "testname", SMALL_INPUT);
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
