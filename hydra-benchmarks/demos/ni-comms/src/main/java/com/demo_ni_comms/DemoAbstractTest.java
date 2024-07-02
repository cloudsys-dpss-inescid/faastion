package com.demo_ni_comms;

public abstract class DemoAbstractTest {

    protected long[] start;
    protected long[] stop;

    protected abstract void printResults(int warmupRuns, int runs);

    public abstract void openSocket(int bufSize, int runs, int warmupRuns);

    public abstract void openSSLSocket(int bufSize, int runs, int warmupRuns);

    public abstract void httpGet(int bufSize, int runs, int warmupRuns);

    public void tcp1_32B(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(32, bufSize, runs, warmupRuns);
    }

    public void tcp2_64B(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(64, bufSize, runs, warmupRuns);
    }

    public void tcp3_128B(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(128, bufSize, runs, warmupRuns);
    }

    public void tcp4_256B(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(256, bufSize, runs, warmupRuns);
    }

    public void tcp5_512B(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(512, bufSize, runs, warmupRuns);
    }

    public void tcp6_1KB(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(1024, bufSize, runs, warmupRuns);
    }

    public void tcp7_16KB(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(16*1024, bufSize, runs, warmupRuns);
    }

    public void tcp7_32KB(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(32*1024, bufSize, runs, warmupRuns);
    }

    public void tcp8_256KB(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(256*1024, bufSize, runs, warmupRuns);
    }

    public void tcp9_512KB(int bufSize, int runs, int warmupRuns) {
        tcpCommsTest(512*1024, bufSize, runs, warmupRuns);
    }

    protected abstract void tcpCommsTest(int size, int bufSize, int runs, int warmupRuns);


    public void udp1_32B(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(32, bufSize, runs, warmupRuns);
    }

    public void udp2_64B(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(64, bufSize, runs, warmupRuns);
    }

    public void udp3_128B(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(128, bufSize, runs, warmupRuns);
    }

    public void udp4_256B(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(256, bufSize, runs, warmupRuns);
    }

    public void udp5_512B(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(512, bufSize, runs, warmupRuns);
    }

    public void udp6_1KB(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(1024, bufSize, runs, warmupRuns);
    }

    public void udp7_32KB(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(32*1024, bufSize, runs, warmupRuns);
    }

    public void udp8_256KB(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(256*1024, bufSize, runs, warmupRuns);
    }

    public void udp9_512KB(int bufSize, int runs, int warmupRuns) {
        udpCommsTest(512*1024, bufSize, runs, warmupRuns);
    }

    protected abstract void udpCommsTest(int size, int bufSize, int runs, int warmupRuns);

}
