package org.graalvm.argo.graalvisor.sandboxing;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;

public class IsolateSandboxHandle extends SandboxHandle {

    private final IsolateSandboxProvider isProvider;

    private final IsolateThread isolateThread;

    public IsolateSandboxHandle(IsolateSandboxProvider isProvider, IsolateThread isolateThread) {
        this.isProvider = isProvider;
        this.isolateThread = isolateThread;
        NativeSandboxInterface.createNativeIsolateSandbox(((NativeFunction) isProvider.getFunction()).hasLazyIsolation());
    }

    public IsolateThread getIsolateThread() {
        return isolateThread;
    }

    @Override
    public String invokeSandbox(String jsonArguments) throws Exception {
        PolyglotFunction function = isProvider.getFunction();
        String functionName = function.getName();
        NativeSandboxInterface.setupMemIsolation(functionName);
        String resp = isProvider.getGraalvisorAPI().invokeFunction((IsolateThread) isolateThread, function.getEntryPoint(), jsonArguments);
        NativeSandboxInterface.teardownMemIsolation(functionName);
        return resp;
    }

    @Override
    public String toString() {
        return Long.toString(Isolates.getIsolate(isolateThread).rawValue());
    }
}