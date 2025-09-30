package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public class ProcessSandboxProvider extends SandboxProvider {

    /**
     * The process sandbox provider loads the function so that child processes
     * can benefit form COW memory.
     */
    public ProcessSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    @Override
    public void loadProvider() throws IOException {
        String fpath = ((NativeFunction) getFunction()).getPath();
        this.functionHandle = NativeSandboxInterface.loadFunction(fpath);
    }

    @Override
    public synchronized String warmupProvider(int concurrency, int requests, String jsonArguments) throws IOException {
        if (concurrency > 1 || requests > 1) {
            return "Error': Warmup operation not supported with multiple threads and requests.";
        }

        long iThreadHandle = NativeSandboxInterface.createSandbox(functionHandle);
        String result = NativeSandboxInterface.invokeSandbox(functionHandle, iThreadHandle, jsonArguments);
        NativeSandboxInterface.destroySandbox(functionHandle, iThreadHandle);
        NativeSandboxInterface.unloadFunction(functionHandle);
        this.loadProvider();
        return result;
    }

    @Override
    public SandboxHandle createSandbox()  throws IOException {
        return new ProcessSandboxHandle(this);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) throws IOException {
        ((ProcessSandboxHandle) shandle).destroyHandle();
    }

    @Override
    public void unloadProvider() throws IOException {
        NativeSandboxInterface.unloadFunction(this.functionHandle);
    }

    @Override
    public String getName() {
        return "process";
    }
}
