package org.graalvm.argo.graalvisor.sandboxing;

import java.io.IOException;

import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;

import com.oracle.svm.graalvisor.api.GraalVisorAPI;

public class RuntimeSandboxHandle extends SandboxHandle {

    private final RuntimeSandboxProvider rsProvider;

    private final GraalVisorAPI graalvisorAPI;

    private final IsolateThread isolateThread;

    public RuntimeSandboxHandle(RuntimeSandboxProvider rsProvider) throws IOException {
        NativeFunction function = (NativeFunction) rsProvider.getFunction();
        this.graalvisorAPI = new GraalVisorAPI(function.getPath());
        this.isolateThread = graalvisorAPI.createIsolate();
        this.rsProvider = rsProvider;
        NativeSandboxInterface.createNativeRuntimeSandbox(((NativeFunction) rsProvider.getFunction()).hasLazyIsolation());
    }

    @Override
    public String invokeSandbox(String jsonArguments) throws Exception {
        PolyglotFunction function = rsProvider.getFunction();
        return graalvisorAPI.invokeFunction((IsolateThread) isolateThread, function.getEntryPoint(), jsonArguments);
    }

    @Override
    public void destroyHandle() throws IOException {
        graalvisorAPI.tearDownIsolate((IsolateThread) isolateThread);
        graalvisorAPI.close();
    }

    @Override
    public String toString() {
        return Long.toString(Isolates.getIsolate(isolateThread).rawValue());
    }

}
