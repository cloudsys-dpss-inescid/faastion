package com.demo_ni_osdcomms;

import java.util.*;
import java.util.function.*;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.Authenticator;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.URI;
import java.net.http.*;
import java.net.http.HttpRequest.BodyPublishers;
import java.net.http.HttpResponse.BodyHandlers;
import java.nio.file.Paths;
import java.time.Duration;
import java.util.Arrays;

public class DemoClient extends DemoAbstractTest {

    public void gson(int bufSize, int runs, int warmupRuns) {
        json = new DemoGsonSerializer();
        send(() -> new DemoObjects.BigObj(), runs, warmupRuns);
    }

    public void jackson(int bufSize, int runs, int warmupRuns) {
        json = new DemoJacksonSerializer();
        send(() -> new DemoObjects.BigObj(), runs, warmupRuns);
    }

    public void kryo(int bufSize, int runs, int warmupRuns) {
        json = new DemoKryoSerializer();
        send(() -> new DemoObjects.BigObj(), runs, warmupRuns);
    }

    private void send(Supplier<Object> objectSupplier, int runs, int warmupRuns) {
        DemoLog.log("[serializer]: processing object of type " + objectSupplier.get().getClass().getName());

        prepareTest(objectSupplier, warmupRuns, runs);

        try {
            for (int i = 0; i < runs + warmupRuns; i++) {
                sendImpl(i);
                Thread.sleep(10);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }

        consumeResults();
        printResults(warmupRuns, runs);
    }

    private void sendImpl(int i) throws Exception {
        start[i] = System.nanoTime();
        serializeImpl(i);
        httpImpl(i);
        deserializeImpl(i);
        stop[i] = System.nanoTime();
    }

    private void serializeImpl(int i) {
        results[i] = json.serialize(toSerialize[i]);
    }

    private void deserializeImpl(int i) {
        results[i] = json.deserialize(toDeserialize[i].toString(), toSerialize[i].getClass());
    }

    private void httpImpl(int i) throws Exception {
        HttpClient client = HttpClient.newBuilder()
            .version(HttpClient.Version.HTTP_1_1)
            .followRedirects(HttpClient.Redirect.NORMAL)
            .connectTimeout(Duration.ofSeconds(1))
            .build();

        HttpRequest request = HttpRequest.newBuilder()
            .uri(URI.create("http://localhost:12345"))
            .timeout(Duration.ofSeconds(1))
            .POST(HttpRequest.BodyPublishers.ofString(results[i].toString()))
            .build();
        HttpResponse<String> response = client.send(request, BodyHandlers.ofString());
        results[i] = response.body();
    }

    protected void printResults(int warmupRuns, int runs) {
        for (int i = 0; i < runs; i++) {
            DemoLog.log(start[warmupRuns + i], "[client]: sending #" + i);
            DemoLog.log(stop[warmupRuns + i], "[client]: sent #" + i);
        }
    }
}
