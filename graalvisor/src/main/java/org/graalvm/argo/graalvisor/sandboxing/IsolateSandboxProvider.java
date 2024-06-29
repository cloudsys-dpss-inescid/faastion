package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.nativeimage.IsolateThread;

import com.oracle.svm.graalvisor.api.GraalVisorAPI;

public class IsolateSandboxProvider extends SandboxProvider {

    private GraalVisorAPI graalvisorAPI;

    public IsolateSandboxProvider(PolyglotFunction function) {
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
    public SandboxHandle createSandbox() {
        IsolateThread isolateThread = graalvisorAPI.createIsolate();
        return new IsolateSandboxHandle(this, isolateThread);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) {
        IsolateSandboxHandle ipshandle = (IsolateSandboxHandle) shandle;
        graalvisorAPI.tearDownIsolate((IsolateThread) ipshandle.getIsolateThread());
    }

    @Override
    public void unloadProvider() throws IOException {
        graalvisorAPI.close();
    }

    @Override
    public String getName() {
        return "isolate";
    }
}
