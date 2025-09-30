package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public class SnapshotProcessSandboxProvider extends SnapshotSandboxProvider {

    public SnapshotProcessSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    @Override
    public synchronized SandboxHandle createSandbox() throws IOException {
        if (warmedUp.get()) {
            if (sandboxHandleCounter.getAndIncrement() == 0) {
                return this.sandboxHandle;
            } else {
                SnapshotSandboxHandle newSandboxHandle = new SnapshotSandboxHandle();
                NativeSandboxInterface.svmRestore(svmID, newSandboxHandle, functionPath, metaSnapPath, memSnapPath);
                return newSandboxHandle;
            }
        } else {
            throw new IOException(String.format("Snapshot sandbox provider not initialized for function %s", function.getName()));
        }
    }

    @Override
    public synchronized void destroySandbox(SandboxHandle shandle) throws IOException {
        if (sandboxHandleCounter.decrementAndGet() == 0) {
            unloadProvider();
        } else {
            NativeSandboxInterface.svmUnload((SnapshotSandboxHandle)shandle);
        }
        shandle.destroyHandle();
    }

    @Override
    public String getName() {
        return "snapshot-process";
    }
}
