package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;
import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public class IsolateSandboxProvider extends SandboxProvider {

    public IsolateSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    @Override
    public void loadProvider() throws IOException {
        String fpath = ((NativeFunction) getFunction()).getPath();
        this.functionHandle = NativeSandboxInterface.loadFunction(fpath);
    }

    @Override
    public SandboxHandle createSandbox() {
        long ithreadPtr = NativeSandboxInterface.createSandbox(functionHandle);
        return new IsolateSandboxHandle(functionHandle, ithreadPtr);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) throws IOException {
        IsolateSandboxHandle ishandle = (IsolateSandboxHandle) shandle;
        NativeSandboxInterface.destroySandbox(functionHandle, ishandle.getIThreadHandle());
        ishandle.destroyHandle();
    }

    @Override
    public void unloadProvider() throws IOException {
        NativeSandboxInterface.unloadFunction(this.functionHandle);
    }

    @Override
    public String getName() {
        return "isolate";
    }
}
