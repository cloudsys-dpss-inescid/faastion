package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;
import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public class ContextSandboxProvider extends SandboxProvider {

    // Context sandboxes use a single isolate. This is the handle to that isolate.
    private long isolateHandle;

    public ContextSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    @Override
    public void loadProvider() throws IOException {
        String fpath = ((NativeFunction) getFunction()).getPath();
        functionHandle = NativeSandboxInterface.loadFunction(fpath);
        long ithreadHandle = NativeSandboxInterface.createSandbox(functionHandle);
        isolateHandle = NativeSandboxInterface.getSandbox(functionHandle, ithreadHandle);
        NativeSandboxInterface.detachThread(functionHandle, ithreadHandle);
    }

    @Override
    public SandboxHandle createSandbox() throws IOException {
        long iThreadHandle = NativeSandboxInterface.attachThread(functionHandle, isolateHandle);
        return new ContextSandboxHandle(functionHandle, iThreadHandle);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) throws IOException {
        NativeSandboxInterface.detachThread(functionHandle, ((ContextSandboxHandle)shandle).getIThreadHandle());
        shandle.destroyHandle();
    }

    @Override
    public void unloadProvider() throws IOException {
        long ithreadHandle = NativeSandboxInterface.attachThread(functionHandle, isolateHandle);
        NativeSandboxInterface.destroySandbox(functionHandle, ithreadHandle);
        NativeSandboxInterface.unloadFunction(functionHandle);
    }

    @Override
    public String getName() {
        return "context";
    }
}
