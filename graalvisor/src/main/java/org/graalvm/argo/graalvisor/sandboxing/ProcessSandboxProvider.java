package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.nativeimage.IsolateThread;

import com.oracle.svm.graalvisor.api.GraalVisorAPI;

public class ProcessSandboxProvider extends SandboxProvider {

    /**
     * The process sandbox provider loads the function so that child processes can benefit form
     * COW memory.
     */
    private GraalVisorAPI graalvisorAPI;

    public ProcessSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    public GraalVisorAPI getGraalvisorAPI() {
        return this.graalvisorAPI;
    }

    @Override
    public void loadProvider() throws IOException {
        this.graalvisorAPI = new GraalVisorAPI(((NativeFunction) getFunction()).getPath());
    }

    @Override
    public synchronized String warmupProvider(String jsonArguments) throws IOException {
        IsolateThread isolateThread = graalvisorAPI.createIsolate();
        String result = graalvisorAPI.invokeFunction((IsolateThread) isolateThread, getFunction().getEntryPoint(), jsonArguments);
        graalvisorAPI.tearDownIsolate(isolateThread);
        graalvisorAPI.close();
        this.loadProvider();
        return result;
    }

    @Override
    public SandboxHandle createSandbox()  throws Exception {
        return new ProcessSandboxHandle(this);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) throws IOException {
        ((ProcessSandboxHandle) shandle).destroyHandle();
    }

    @Override
    public void unloadProvider() throws IOException {
        graalvisorAPI.close();
    }

    @Override
    public String getName() {
        return "process";
    }
}
