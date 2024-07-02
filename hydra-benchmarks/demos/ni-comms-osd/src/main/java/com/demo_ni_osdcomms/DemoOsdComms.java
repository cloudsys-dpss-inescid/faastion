package com.demo_ni_osdcomms;

import java.lang.reflect.Method;

public class DemoOsdComms {
    public static void main(String[] args) throws Exception {
        Object receiver = null;
        Method testMethod = null;
        String testName = null;
        int runs = 1000;
        int warmupRuns = 1;
        int bufSize = 8192;

        for (int i = 0; i < args.length; i += 2) {
            switch (args[i]) {
                case "--client":
                    receiver = new DemoClient();
                    testName = args[i+1];
                    testMethod = DemoClient.class.getMethod(testName, int.class, int.class, int.class);
                    break;
                case "--server":
                    receiver = new DemoServer();
                    testName = args[i+1];
                    testMethod = DemoServer.class.getMethod(testName, int.class, int.class, int.class);
                    break;
                case "--runs":
                    runs = Integer.parseInt(args[i+1]);
                    break;
                case "--warmup":
                    warmupRuns = Integer.parseInt(args[i+1]);
                    break;
                case "--bufsize":
                    bufSize = Integer.parseInt(args[i+1]);
                    break;
                default:
                    usageError();
            }
        }

        if (receiver == null || testMethod == null) {
            usageError();
        }

        DemoLog.log(String.format("[harness]: invoking %s , warmup=%d, runs=%d, bufsize=%d", testName, warmupRuns, runs, bufSize));
        testMethod.invoke(receiver, bufSize, runs, warmupRuns);
    }

    private static void usageError() {
        System.err.println("usage: ./bench (--server | --client) test_case_name [--runs num_runs] [--warmup warmup_runs]");
        System.exit(1);
    }
}
