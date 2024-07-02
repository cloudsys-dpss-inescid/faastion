package com.demo_ni_comms;

public class DemoLog {

    public static void log(String msg) {
        log(System.nanoTime(), msg);
    }

    public static void log(long time, String msg) {
        System.out.printf("[%d]: %s\n", time, msg);
    }
    
}
