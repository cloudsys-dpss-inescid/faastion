package com.uploader;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Map;
import java.util.HashMap;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;
import java.io.FileOutputStream;

import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.mime.FormBodyPart;
import org.apache.http.entity.mime.content.FileBody;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.entity.mime.MultipartEntityBuilder;

import org.apache.http.entity.FileEntity;
import org.apache.http.entity.ContentType;

import java.util.concurrent.ThreadLocalRandom;

public class Uploader {

    private static final String url = "http://172.18.0.1:8000/snap.png";
    private static final String upload_url = "http://172.18.0.1:9696/upload";
    public static String IMG_FILENAME = String.format("img-%d.png", ThreadLocalRandom.current().nextInt(0, 1024 + 1));

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

    public static int uploadFile(String url, String tmpDir, String filePath) throws IOException {
        HttpClient httpclient = new DefaultHttpClient();
        HttpPost httppost = new HttpPost(url);

        File file = new File(filePath);
        Path path = Paths.get(tmpDir);
        Path fileName = path.getFileName();
        HttpEntity entity = MultipartEntityBuilder.create()
                .addBinaryBody(
                    "file",
                    file,
                    ContentType.create("image/png"),
                    fileName + ".png" // System.currentTimeMillis() + ".png"
                )
                .build();
        httppost.setEntity(entity);

        HttpResponse response = httpclient.execute(httppost);
        return response.getStatusLine().getStatusCode();
    }

    public static HashMap<String, Object> main(Map<String, Object> input) {
        String tmpDir = (String) input.get("tmpDir");
        String filePath = tmpDir + "/" + IMG_FILENAME;

        HashMap<String, Object> output = new HashMap<>();

        int result = 400;
        boolean success;
        if ((success = downloadFile(url, filePath))) {
            try {
                result = uploadFile(upload_url, tmpDir, filePath);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        output.put("result", result);

        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }
}
