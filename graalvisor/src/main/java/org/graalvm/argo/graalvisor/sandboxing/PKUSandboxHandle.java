package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.Main;

public class PKUSandboxHandle extends SandboxHandle {

    // Native function handle (pointer casted to long).
    private final long functionHandle;
    // Native isolate thread handle (pointer casted to long).
    private final long iThreadHandle;

    public PKUSandboxHandle(long functionHandle, long iThreadHandle) {
        this.functionHandle = functionHandle;
        this.iThreadHandle = iThreadHandle;
        NativeSandboxInterface.createNativePKUSandbox();
    }

    public long getIThreadHandle() {
        return this.iThreadHandle;
    }

    @Override
    public String invokeSandbox(String jsonArguments) throws IOException {
        return NativeSandboxInterface.invokeSandbox(functionHandle, iThreadHandle, jsonArguments);
    }

    @Override
    public void destroyHandle() throws IOException {
        super.destroyHandle();
        NativeSandboxInterface.teardownNativePKUSandbox();
    }

    @Override
    public String toString() {
        return Long.toString(iThreadHandle);
    }

    public boolean supportsLPI() {
        return PKUSandboxProvider.LPI;
    }
}