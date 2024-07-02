package com.demo.sharedmemory;

import java.io.File;
import java.io.IOException;

public class SendOnlySharedMemoryChannel extends SharedMemoryChannel {

    public SendOnlySharedMemoryChannel(String path) throws IOException {
        super(new File(path).toPath());
        buffer.put(0, readyToWrite);
    }

    public void writeString(String s) throws InterruptedException {
        buffer.clear();
        while(buffer.get(0) != readyToWrite) {
            Thread.sleep(0, 100);
        }
        buffer.put(readyToWrite); // Advance the first char.

        buffer.put(s.toCharArray());
        buffer.put('\0');
        buffer.put(0, readyToRead);
    }

}
