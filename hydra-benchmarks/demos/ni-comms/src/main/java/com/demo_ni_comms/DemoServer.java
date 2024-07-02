package com.demo_ni_comms;

import java.io.BufferedInputStream;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.net.Authenticator;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.http.HttpClient;
import java.time.Duration;

public class DemoServer extends DemoAbstractTest {

    public static int PORT = 12345;

    public void openSocket(int bufSize, int runs, int warmupRuns) {
        start = new long[runs + warmupRuns];
        stop = new long[runs + warmupRuns];
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            for (int i = 0; i < runs + warmupRuns; i++) {
                openSocketImpl(serverSocket, i);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        printResults(warmupRuns, runs);
    }

    public void openSSLSocket(int bufSize, int runs, int warmupRuns) {
        // nop
    }

    private void openSocketImpl(ServerSocket server, int i) {
        try {
            start[i] = System.nanoTime();
            Socket client = server.accept();
            stop[i] = System.nanoTime();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    public void httpGet(int bufSize, int runs, int warmupRuns) {
        start = new long[runs + warmupRuns];
        stop = new long[runs + warmupRuns];
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            for (int i = 0; i < runs + warmupRuns; i++) {
                httpGetImpl(serverSocket, i);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        printResults(warmupRuns, runs);
    }

    private void httpGetImpl(ServerSocket server, int i) {
        try {
            String response = 
                "HTTP/1.1 200 OK\r\n" +
                "Content-Length: 88\r\n" +
                "Content-Type: text/html\r\n" +
                "\r\n" +
                "<html>\r\n" +
                "<body>\r\n" +
                "<h1>Hello, World!</h1>\r\n" +
                "</body>\r\n" +
                "</html>\r\n"
                ;
            start[i] = System.nanoTime();
            Socket client = server.accept();
            System.err.println("Accepted");
            client.getInputStream().readAllBytes();
            try (BufferedWriter w = new BufferedWriter(new OutputStreamWriter(client.getOutputStream()))) {
                w.write(response);
                w.flush();
            }
            stop[i] = System.nanoTime();
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    protected void tcpCommsTest(int size, int bufSize, int runs, int warmupRuns) {
        start = new long[runs + warmupRuns];
        stop = new long[runs + warmupRuns];

        DemoLog.log(String.format("[server]: starting on localhost:%d...", DemoServer.PORT));    
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            Socket client = serverSocket.accept();
            try (InputStream in = new BufferedInputStream(client.getInputStream(), bufSize == 0 ? size : bufSize)) {
                for (int i = 0; i < warmupRuns + runs; i++) {
                    tcpCommsTestRecvData(in, i, size);
                }
                printResults(warmupRuns, runs);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void tcpCommsTestRecvData(InputStream in, int i, int size) throws IOException {
        start[i] = System.nanoTime();
        byte[] received = in.readNBytes(size);
        stop[i] = System.nanoTime();
        assert received != null;
        assert received.length == size;
    }

    protected void udpCommsTest(int size, int bufSize, int runs, int warmupRuns) {
        start = new long[runs + warmupRuns];
        stop = new long[runs + warmupRuns];

        try (DatagramSocket socket = new DatagramSocket(DemoServer.PORT)) {
            socket.setSoTimeout(5000);
            byte[] bytes = new byte[size];
            DatagramPacket datagram = new DatagramPacket(bytes, size);

            for (int i = 0; i < runs + warmupRuns; i++) {
                udpCommsTestSendData(socket, datagram, i, size);
            }

            printResults(warmupRuns, runs);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void udpCommsTestSendData(DatagramSocket socket, DatagramPacket datagram, int i, int size) throws IOException {
        start[i] = System.nanoTime();
        socket.receive(datagram);
        stop[i] = System.nanoTime();
        assert datagram.getData() != null;
        assert datagram.getLength() == size;
    }

    protected void printResults(int warmupRuns, int runs) {
        for (int i = 0; i < runs; i++) {
            DemoLog.log(start[warmupRuns + i], "[server]: receiving #" + i);
            DemoLog.log(stop[warmupRuns + i], "[server]: received #" + i);
        }
    }
}
