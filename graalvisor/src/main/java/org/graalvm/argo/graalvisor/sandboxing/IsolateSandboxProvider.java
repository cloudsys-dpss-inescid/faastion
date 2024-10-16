package org.graalvm.argo.graalvisor.sandboxing;

import org.graalvm.argo.graalvisor.Main;
import org.graalvm.argo.graalvisor.RuntimeProxy;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.nativeimage.IsolateThread;

import com.oracle.svm.graalvisor.api.GraalVisorAPI;

public class IsolateSandboxProvider extends SandboxProvider {

    private GraalVisorAPI graalvisorAPI;
    private boolean LPI;
    

    public IsolateSandboxProvider(PolyglotFunction function) {
        super(function);
        String enableLPI = System.getenv("LPI");
        LPI = enableLPI == null ? false : enableLPI.equals("true");
    }

    public GraalVisorAPI getGraalvisorAPI() {
        return this.graalvisorAPI;
    }

    @Override
    public PolyglotFunction getQualifiedFuncion() {
        String processFunctionName;
        PolyglotFunction qualifiedFunction = null;

        if (LPI && NativeSandboxInterface.resetActiveWaitingCount(Main.ACTIVE_WAIT_CAP)) {
            processFunctionName = getFunction().getName().replaceAll("[\\d.]", "");
            qualifiedFunction = RuntimeProxy.FTABLE.get(processFunctionName);
        }

        if (qualifiedFunction == null) {
            qualifiedFunction = getFunction();
        }

        return qualifiedFunction;
    }

    @Override
    public void loadProvider() throws IOException {
        this.graalvisorAPI = new GraalVisorAPI(((NativeFunction) getFunction()).getPath());
        NativeSandboxInterface.createIsolateFunction(((NativeFunction) getFunction()).getName());
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
        NativeSandboxInterface.destroyIsolateFunction(((NativeFunction) getFunction()).getName());
    }

    @Override
    public String getName() {
        return "isolate";
    }
}
