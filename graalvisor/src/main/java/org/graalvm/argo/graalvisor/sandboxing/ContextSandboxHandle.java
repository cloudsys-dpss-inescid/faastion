package org.graalvm.argo.graalvisor.sandboxing;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.nativeimage.IsolateThread;

public class ContextSandboxHandle extends SandboxHandle {

    private final ContextSandboxProvider provider;

    private final IsolateThread isolateThread;

    public ContextSandboxHandle(ContextSandboxProvider provider, IsolateThread isolateThread) {
        this.provider = provider;
        this.isolateThread = isolateThread;
    }

    public IsolateThread getIsolateThread() {
        return isolateThread;
    }

    @Override
    public String invokeSandbox(String jsonArguments) throws Exception {
        PolyglotFunction function = provider.getFunction();
        return provider.getGraalvisorAPI().invokeFunction((IsolateThread) isolateThread, function.getEntryPoint(), jsonArguments);
    }

    @Override
    public String toString() {
        return Long.toString(provider.getGraalvisorAPI().getIsolate(isolateThread).rawValue());
    }
}