package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public abstract class SandboxProvider {

    // The function that this provider if serving.
    protected final PolyglotFunction function;

    // Native function handle (pointer casted to long).
    protected long functionHandle;


    public SandboxProvider(PolyglotFunction function) {
        this.function = function;
    }

    public PolyglotFunction getFunction() {
        return this.function;
    }

    public long getFunctionHandle() {
        return this.functionHandle;
    }

    public abstract String getName();

    public abstract void loadProvider() throws IOException;

    public String warmupProvider(int concurrency, int requests, String jsonArguments) throws IOException {
        return String.format("{'Error': 'Provider %s has no support for warmup operation'}", this.getName());
    }

    public boolean isWarm() {
        return true;
    }

    public abstract SandboxHandle createSandbox() throws IOException;

    public abstract void destroySandbox(SandboxHandle shandle) throws IOException;

    public abstract void unloadProvider() throws IOException;

}