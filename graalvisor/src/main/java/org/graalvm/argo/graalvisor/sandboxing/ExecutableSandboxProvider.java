package org.graalvm.argo.graalvisor.sandboxing;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;

import java.io.IOException;

public class ExecutableSandboxProvider extends SandboxProvider {
    private final String appDir;
    public ExecutableSandboxProvider(PolyglotFunction function, final String appDir) {
        super(function);
        this.appDir = appDir;
    }

    public String getAppDir() {
        return appDir;
    }

    @Override
    public String getName() {
        return "pgo";
    }

    @Override
    public void loadProvider() throws IOException {

    }

    @Override
    public SandboxHandle createSandbox() throws IOException {
        return new ExecutableSandboxHandle(this);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) throws IOException {
        shandle.destroyHandle();
    }

    @Override
    public void unloadProvider() throws IOException {

    }
}
