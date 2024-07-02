package org.graalvm.argo.dataset.utils;

import java.util.Arrays;

public class RingBuffer {

    private final int[] buffer;
    private int writeSequence;
    private final int capacity;

    public RingBuffer(int capacity) {
        this.buffer = new int[capacity];
        this.capacity = capacity;
        this.writeSequence = -1;
    }

    public void offer(int element) {
        buffer[++writeSequence % capacity] = element;
    }

    public int readOldest() {
        return buffer[(writeSequence + 1) % capacity];
    }

    @Override
    public String toString() {
        return Arrays.toString(buffer) + "; writeSequence = " + writeSequence;
    }
}
