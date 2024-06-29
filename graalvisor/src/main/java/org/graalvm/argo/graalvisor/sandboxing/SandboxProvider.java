package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public abstract class SandboxProvider {

    private final PolyglotFunction function;

    public SandboxProvider(PolyglotFunction function) {
        this.function = function;
    }

    public PolyglotFunction getFunction() {
        return this.function;
    }

    public abstract String getName();

    public abstract void loadProvider() throws IOException;

    public String warmupProvider(String jsonArguments) throws IOException {
        return String.format("{'Error': 'Provider %s has no support for warmup operation'}", this.getName());
    }

    public abstract SandboxHandle createSandbox() throws Exception;

    public abstract void destroySandbox(SandboxHandle shandle) throws IOException;

    public abstract void unloadProvider() throws IOException;

}