package org.graalvm.argo.dataset.utils;

import java.util.HashMap;
import java.util.HashSet;

public class ColdStartSlidingWindow {

    private final int capacity;
    private final int period;
    private final HashMap<String, RingBuffer> functions;
    private final HashSet<String> optimizedFunctions;

    public ColdStartSlidingWindow(int capacity, int period) {
        this.capacity = capacity;
        this.period = period;
        this.functions = new HashMap<>();
        this.optimizedFunctions = new HashSet<>();
    }

    public void add(String function, int timestamp) {
        RingBuffer buffer;
        if (!functions.containsKey(function)) {
            buffer = new RingBuffer(capacity);
            functions.put(function, buffer);
        } else {
            buffer = functions.get(function);
        }
        buffer.offer(timestamp);
    }

    public boolean optimized(String function) {
        return optimizedFunctions.contains(function);
    }

    public boolean worthOptimizing(String function, int currentTimestamp) {
        if (!functions.containsKey(function)) {
            return false;
        }
        RingBuffer buffer = functions.get(function);
        int oldestTimestamp = buffer.readOldest();
        if (oldestTimestamp >= (currentTimestamp - period)) {
            functions.remove(function);
            optimizedFunctions.add(function);
            return true;
        } else {
            return false;
        }
    }
}
