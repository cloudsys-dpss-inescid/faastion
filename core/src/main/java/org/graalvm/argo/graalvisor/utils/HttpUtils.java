package org.graalvm.argo.graalvisor.utils;

import java.io.ByteArrayOutputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.URL;
import java.nio.channels.Channels;
import java.nio.channels.ReadableByteChannel;
import java.nio.charset.StandardCharsets;
import com.sun.net.httpserver.HttpExchange;
import java.util.Map;
import java.util.HashMap;

public class HttpUtils {

    public static void writeResponse(HttpExchange t, int code, String response) throws IOException {
        byte[] bytes = response.getBytes(StandardCharsets.UTF_8);
        t.sendResponseHeaders(code, bytes.length);
        OutputStream os = t.getResponseBody();
        os.write(bytes);
        os.close();
    }

    public static void errorResponse(HttpExchange t, String errorMessage) throws IOException {
        writeResponse(t, 502, "ERROR: " + errorMessage);
    }

    public static String extractRequestBody(HttpExchange t) {
        ByteArrayOutputStream result = new ByteArrayOutputStream();
        byte[] buffer = new byte[1024];
        try {
            for (int length; (length = t.getRequestBody().read(buffer)) != -1;) {
                result.write(buffer, 0, length);
            }
            return result.toString(StandardCharsets.UTF_8);
        } catch (IOException e) {
            e.printStackTrace(System.err);
            return "";
        }
    }

    public static Map<String, String> getRequestParameters(String request) {
        String[] splits = request.split("&");
        Map<String, String> params = new HashMap<>();
        for (String param : splits) {
            String[] keyValue = param.split("=");
            params.put(keyValue[0], keyValue[1]);
        }
        return params;
    }

    public static void downloadFile(String url, String localPath) throws IOException {
        ReadableByteChannel rbc = Channels.newChannel(new URL(url).openStream());
        FileOutputStream fos = new FileOutputStream(localPath);
        fos.getChannel().transferFrom(rbc, 0, Long.MAX_VALUE);
        fos.close();
    }
}
