package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

public class SnapshotSandboxHandle extends SandboxHandle {

    /**
     * Native sandbox handle (pointer casted to long).
     * This value is set from JNI (see checkpoint and restore svm).
     */
    private volatile long sandboxHandle = 0;

    @Override
    public String invokeSandbox(String jsonArguments) throws IOException {
        return NativeSandboxInterface.svmInvoke(this, jsonArguments);
    }

    @Override
    public String toString() {
        return Long.toString(sandboxHandle);
    }
}
