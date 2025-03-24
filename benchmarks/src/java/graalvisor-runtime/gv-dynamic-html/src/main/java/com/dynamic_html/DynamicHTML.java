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

import com.github.mustachejava.DefaultMustacheFactory;
import com.github.mustachejava.Mustache;
import com.github.mustachejava.MustacheFactory;

import java.util.List;
import java.util.Date;
import java.util.Random;
import java.text.SimpleDateFormat;
import java.util.stream.Collectors;

public class DynamicHTML {

    private static final String url = "http://127.0.0.1:8000/template.html";
    private static final String filePath = "/tmp/template.html";
    private static final int TEST_INPUT = 10;
    private static final int SMALL_INPUT = 1000;
    private static final int LARGE_INPUT = 100000;

    String username;
    String cur_time;
    List<String> random_numbers;

    public DynamicHTML() {}

    public DynamicHTML(String name, int size) {
        username = name;

        cur_time = new SimpleDateFormat("MM/dd/yyyy HH:mm:ss")
            .format(new Date());

        random_numbers = new Random().ints(size, 0, 101)
            .mapToObj(n -> String.valueOf(n))
            .collect(Collectors.toList());
    }

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

    public static void deleteFile(String filePath) {
        try {
            Path path = Paths.get(filePath);
            Files.deleteIfExists(path);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void renderTemplate(String name, int size) {
        MustacheFactory mf = new DefaultMustacheFactory();
        Mustache mustache = mf.compile(filePath);
        try {
            mustache.execute(new PrintWriter(System.out), new DynamicHTML(name, size)).flush();
        } catch (IOException e) {
            e.printStackTrace();
            System.exit(1);
        }
    }

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();

        boolean success;
        if ((success = downloadFile(url, filePath))) {
            renderTemplate("testname", LARGE_INPUT);
            deleteFile(filePath);
        }
        output.put("success", success);

        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
    }
}
