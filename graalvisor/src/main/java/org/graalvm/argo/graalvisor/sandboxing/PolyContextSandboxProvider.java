package org.graalvm.argo.graalvisor.sandboxing;

import static org.graalvm.argo.graalvisor.utils.IsolateUtils.copyString;
import static org.graalvm.argo.graalvisor.utils.IsolateUtils.retrieveString;

import java.io.IOException;

import org.graalvm.argo.graalvisor.RuntimeProxy;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.argo.graalvisor.function.TruffleFunction;
import org.graalvm.nativeimage.Isolate;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;
import org.graalvm.nativeimage.ObjectHandle;
import org.graalvm.nativeimage.c.function.CEntryPoint;

public class PolyContextSandboxProvider extends SandboxProvider {

    private Isolate isolate;

    public PolyContextSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    @CEntryPoint
    private static void installSourceCode(@CEntryPoint.IsolateThreadContext IsolateThread workingThread,
                    ObjectHandle functionNameHandle,
                    ObjectHandle entryPointHandle,
                    ObjectHandle languageHandle,
                    ObjectHandle sourceCodeHandle) {
        String functionName = retrieveString(functionNameHandle);
        String entryPoint = retrieveString(entryPointHandle);
        String language = retrieveString(languageHandle);
        String sourceCode = retrieveString(sourceCodeHandle);
        RuntimeProxy.FTABLE.put(functionName, new TruffleFunction(functionName, entryPoint, language, sourceCode));
    }

    @Override
    public void loadProvider() throws IOException {
        TruffleFunction tfunction = (TruffleFunction) getFunction();
        IsolateThread isolateThread = Isolates.createIsolate(Isolates.CreateIsolateParameters.getDefault());
        this.isolate = Isolates.getIsolate(isolateThread);
        installSourceCode(isolateThread,
                        copyString(isolateThread, tfunction.getName()),
                        copyString(isolateThread, tfunction.getEntryPoint()),
                        copyString(isolateThread, tfunction.getLanguage().name()),
                        copyString(isolateThread, tfunction.getSource()));
        Isolates.detachThread(isolateThread);
    }

    @Override
    public SandboxHandle createSandbox() {
        IsolateThread isolateThread = Isolates.attachCurrentThread(isolate);
        return new PolyContextSandboxHandle(this, isolateThread);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) {
        Isolates.detachThread(((IsolateSandboxHandle)shandle).getIsolateThread());
    }

    @Override
    public void unloadProvider() throws IOException {
        IsolateThread isolateThread = Isolates.attachCurrentThread(isolate);
        Isolates.tearDownIsolate(isolateThread);
    }

    @Override
    public String getName() {
        return "polycontext";
    }
}
