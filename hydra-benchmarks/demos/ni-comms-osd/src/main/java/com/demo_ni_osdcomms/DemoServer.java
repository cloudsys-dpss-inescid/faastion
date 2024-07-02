package com.demo_ni_osdcomms;

import java.util.*;
import java.util.function.*;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Authenticator;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.http.*;
import java.time.Duration;

public class DemoServer extends DemoAbstractTest {

    public static int PORT = 12345;

    private static String response = 
        "HTTP/1.1 200 OK\r\n" +
        "Server: Sample\r\n" +
        "Last-Modified: Mon, 11 Jun 2007 18:53:14 GMT\r\n" +
        "Content-Length: 5\r\n" +
        "Content-Type: text/plain\r\n" +
        "\r\n" +
        "hello"
        ;

    public void gson(int bufSize, int runs, int warmupRuns) {
        json = new DemoGsonSerializer();
        recv(() -> new DemoObjects.BigObj(), runs, warmupRuns);
    }

    public void jackson(int bufSize, int runs, int warmupRuns) {
        json = new DemoJacksonSerializer();
        recv(() -> new DemoObjects.BigObj(), runs, warmupRuns);
    }

    public void kryo(int bufSize, int runs, int warmupRuns) {
        json = new DemoKryoSerializer();
        recv(() -> new DemoObjects.BigObj(), runs, warmupRuns);
    }

    private void recv(Supplier<Object> objectSupplier, int runs, int warmupRuns) {
        DemoLog.log("[serializer]: processing object of type " + objectSupplier.get().getClass().getName());

        prepareTest(objectSupplier, warmupRuns, runs);
        Class<?> clazz = objectSupplier.get().getClass();

        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            for (int i = 0; i < runs + warmupRuns; i++) {
                try (Socket client = serverSocket.accept()) {
                    recvImpl(client, i, clazz);
                }
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

        consumeResults();
        printResults(warmupRuns, runs);
    }

    private void recvImpl(Socket client, int i, Class<?> clazz) throws Exception {
        start[i] = System.nanoTime();
        String jsonStr = httpImpl(client);
        deserializeImpl(jsonStr, i, clazz);
        stop[i] = System.nanoTime();
    }

    private void deserializeImpl(String jsonStr, int i, Class<?> clazz) {
        results[i] = json.deserialize(jsonStr, clazz);
    }

    private String httpImpl(Socket client) throws Exception {
        StringBuilder sb = new StringBuilder();
        boolean body = false;
        try (BufferedReader br = new BufferedReader(new InputStreamReader(client.getInputStream()));
             BufferedWriter bw = new BufferedWriter(new OutputStreamWriter(client.getOutputStream()))
        ) {
            String line;
            while ((line = br.readLine()) != null) {
                if (body) {
                    sb.append(line);
                    sb.append("\r\n");
                } else {
                    if (line.trim().isEmpty()) {
                        body = true;
                    }
                }
                if (line.endsWith("}")) {
                    break;
                }
            }
            System.out.println("Received:");
            System.out.println(sb.toString());
            bw.write(response);
            bw.flush();
        }
        return sb.toString();
    }

    protected void printResults(int warmupRuns, int runs) {
        System.out.println(Arrays.toString(start));
        System.out.println(Arrays.toString(stop));
        for (int i = 0; i < runs; i++) {
            DemoLog.log(start[warmupRuns + i], "[server]: receiving #" + i);
            DemoLog.log(stop[warmupRuns + i], "[server]: received #" + i);
        }
    }
}
