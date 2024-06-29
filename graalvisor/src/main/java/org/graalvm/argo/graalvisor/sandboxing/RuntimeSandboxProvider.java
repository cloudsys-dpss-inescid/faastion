package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public class RuntimeSandboxProvider extends SandboxProvider {

    public RuntimeSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    @Override
    public void loadProvider() throws IOException {
        // noop
    }

    @Override
    public SandboxHandle createSandbox()  throws IOException {
        return new RuntimeSandboxHandle(this);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) throws IOException {
        ((RuntimeSandboxHandle) shandle).destroyHandle();
    }

    @Override
    public void unloadProvider() throws IOException {
        // noop
    }

    @Override
    public String getName() {
        return "runtime";
    }
}
