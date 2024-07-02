package com.demo_ni_osd;

import java.lang.reflect.Method;

public class DemoOSD {
    public static void main(String[] args) throws Exception {
        
        // DemoSerializer json = new DemoGsonSerializer();
        // DemoSerializer json = new DemoJacksonSerializer();
        DemoSerializer json = new DemoDsljsonSerializer();
        // DemoSerializer json = new DemoJsoniterSerializer();
        // DemoSerializer json = new DemoKryoSerializer();

        DemoTest receiver = new DemoTest(json);
        Method testMethod = null;
        String testName = null;
        int runs = 1000;
        int warmupRuns = 1;

        for (int i = 0; i < args.length; i += 2) {
            switch (args[i]) {
                case "--runs":
                    runs = Integer.parseInt(args[i+1]);
                    continue;
                case "--warmup":
                    warmupRuns = Integer.parseInt(args[i+1]);
                    continue;
                default:
                    if (testMethod != null) {
                        usageError();
                    } else {
                        testName = args[i];
                        testMethod = DemoTest.class.getMethod(testName, int.class, int.class);
                        i--;
                    }
            }
        }

        if (receiver == null || testMethod == null) {
            usageError();
        }

        DemoLog.log(String.format("[harness]: invoking %s , warmup=%d, runs=%d", testName, warmupRuns, runs));
        testMethod.invoke(receiver, runs, warmupRuns);
        receiver.printResults(warmupRuns, runs);
    }

    private static void usageError() {
        System.err.println("usage: ./bench test_case_name [--runs num_runs] [--warmup warmup_runs]");
        System.exit(1);
    }
}
