package com.oracle.svm.graalvisor.polyglot;

import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.nio.file.Files;
import java.nio.file.StandardOpenOption;
import org.graalvm.polyglot.HostAccess;

public class PolyglotHostAccess {

    @HostAccess.Export
    public void sleep(long milliseconds) {
        try {
            Thread.sleep(milliseconds);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @HostAccess.Export
    public byte[] readBytes(String path) {
        try {
            return Files.readAllBytes(new File(path).toPath());
        } catch (Exception e) {
            e.printStackTrace();
            return new byte[0];
        }
    }

    @HostAccess.Export
    public void writeBytes(byte[] bytes, String path) {
        try {
            Files.write(new File(path).toPath(), bytes, StandardOpenOption.CREATE);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @HostAccess.Export
    public byte[] downloadBytes(String url) {
        try {
            return downloadBytesInternal(url);
        } catch (Exception e) {
            e.printStackTrace();
            return e.getMessage().getBytes();
        }
    }

    public static byte[] downloadBytesInternal(String url) throws Exception {
        URLConnection conn = new URL(url).openConnection();
        try (InputStream is = conn.getInputStream()) {
            return is.readAllBytes();
        }
    }


    @HostAccess.Export
    public void uploadBytes(String url, byte[] bytes) {
        try {
            int code = uploadBytesInternal(url, bytes);
            System.out.println(code); // Should be 200
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Based on https://stackoverflow.com/questions/2469451/upload-files-from-java-client-to-a-http-server
    public static int uploadBytesInternal(String url, byte[] bytes) throws Exception {
        String boundary = Long.toHexString(System.currentTimeMillis()); // Just generate some unique random value.
        String CRLF = "\r\n"; // Line separator required by multipart/form-data.
        String charset = "UTF-8";
        URLConnection connection = new URL(url).openConnection();
        connection.setDoOutput(true);
        connection.setRequestProperty("Content-Type", "multipart/form-data; boundary=" + boundary);

        try (
            OutputStream output = connection.getOutputStream();
            PrintWriter writer = new PrintWriter(new OutputStreamWriter(output, charset), true);
        ) {
            // Send binary file.
            writer.append("--" + boundary).append(CRLF);
            writer.append("Content-Disposition: form-data; name=\"binaryFile\"; filename=\"myimage.png\"").append(CRLF);
            writer.append("Content-Type: png").append(CRLF);
            writer.append("Content-Transfer-Encoding: binary").append(CRLF);
            writer.append(CRLF).flush();
            output.write(bytes);
            output.flush(); // Important before continuing with writer!
            writer.append(CRLF).flush(); // CRLF is important! It indicates end of boundary.

            // End of multipart/form-data.
            writer.append("--" + boundary + "--").append(CRLF).flush();
        }

        // Request is lazily fired whenever you need to obtain information about response.
        int code = ((HttpURLConnection) connection).getResponseCode();
        ((HttpURLConnection) connection).disconnect();
        return code;
    }
}
