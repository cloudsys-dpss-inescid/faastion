package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.nativeimage.Isolate;
import org.graalvm.nativeimage.IsolateThread;
import com.oracle.svm.graalvisor.api.GraalVisorAPI;

public class ContextSandboxProvider extends SandboxProvider {

    private GraalVisorAPI graalvisorAPI;

    private Isolate isolate;

    public ContextSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    public GraalVisorAPI getGraalvisorAPI() {
        return this.graalvisorAPI;
    }

    @Override
    public void loadProvider() throws IOException {
        this.graalvisorAPI = new GraalVisorAPI(((NativeFunction) getFunction()).getPath());
        IsolateThread isolateThread = graalvisorAPI.createIsolate();
        this.isolate = graalvisorAPI.getIsolate(isolateThread);
        graalvisorAPI.detachThread(isolateThread);
    }

    @Override
    public SandboxHandle createSandbox() {
        IsolateThread isolateThread = graalvisorAPI.attachThread(isolate);
        return new ContextSandboxHandle(this, isolateThread);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) {
        graalvisorAPI.detachThread(((ContextSandboxHandle)shandle).getIsolateThread());
    }

    @Override
    public void unloadProvider() throws IOException {
        IsolateThread isolateThread = graalvisorAPI.attachThread(isolate);
        graalvisorAPI.tearDownIsolate(isolateThread);
        graalvisorAPI.close();
    }

    @Override
    public String getName() {
        return "context";
    }
}
