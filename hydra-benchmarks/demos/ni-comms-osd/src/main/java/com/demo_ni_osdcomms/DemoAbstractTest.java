package com.demo_ni_osdcomms;

import java.util.*;
import java.util.function.*;

public abstract class DemoAbstractTest {

    protected Object[] toSerialize;
    protected String[] toDeserialize;
    protected Object[] results;
    protected long[] start;
    protected long[] stop;
    protected DemoSerializer json;

    protected abstract void printResults(int warmupRuns, int runs);

    public abstract void gson(int bufSize, int runs, int warmupRuns);

    public abstract void jackson(int bufSize, int runs, int warmupRuns);

    public abstract void kryo(int bufSize, int runs, int warmupRuns);

    protected void prepareTest(Supplier<Object> objectSupplier, int warmupRuns, int runs) {
        int total = warmupRuns + runs;
        start = new long[total];
        stop = new long[total];
        toSerialize = new Object[total];
        toDeserialize = new String[total];
        results = new Object[total];
        for (int i = 0; i < total; i++) {
            Object o = objectSupplier.get();
            toSerialize[i] = o;
            toDeserialize[i] = json.serialize(o);
        }
    }

    protected void consumeResults() {
        DemoLog.log("[blackhole]: Consuming results: " + Arrays.hashCode(results));
    }
}
