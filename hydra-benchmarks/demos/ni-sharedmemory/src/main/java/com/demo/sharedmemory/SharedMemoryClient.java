package com.demo.sharedmemory;

public class SharedMemoryClient {

    public static void main(String[] args) throws Exception {
        SendOnlySharedMemoryChannel c2s = new SendOnlySharedMemoryChannel("/tmp/shared-client-2-server");
        ReceiveOnlySharedMemoryChannel s2c = new ReceiveOnlySharedMemoryChannel("/tmp/shared-server-2-client");

        for(int i = 0; i < 10; i++) {
            System.out.println("Receiving from server");
            String s = s2c.readString();
            System.out.println("Received from server: " + s);

            s = s.toLowerCase();
            System.out.println("Sending to server: " + s);
            c2s.writeString(s);
            System.out.println("Sent to server: " + s);
        }
    }
}
