package com.demo.sharedmemory;

import java.io.File;
import java.io.IOException;

public class ReceiveOnlySharedMemoryChannel extends SharedMemoryChannel {

    public ReceiveOnlySharedMemoryChannel(String path) throws IOException {
        super(new File(path).toPath());
    }

    public String readString() throws InterruptedException {
        buffer.clear();
        while(buffer.get(0) != readyToRead) {
            Thread.sleep(0, 100);
        }
        buffer.get(); // Advance the first char.

        char c;
        int size = 0;
        while((c = buffer.get()) != '\0') {
            readBuffer[size++] = c;
        }

        buffer.put(0, readyToWrite);
        return new String(readBuffer, 0, size);
    }
}
