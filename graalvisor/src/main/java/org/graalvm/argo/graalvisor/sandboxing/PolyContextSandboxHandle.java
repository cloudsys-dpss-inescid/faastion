package org.graalvm.argo.graalvisor.sandboxing;

import static org.graalvm.argo.graalvisor.utils.IsolateUtils.copyString;
import static org.graalvm.argo.graalvisor.utils.IsolateUtils.retrieveString;

import org.graalvm.argo.graalvisor.RuntimeProxy;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.argo.graalvisor.function.TruffleFunction;
import org.graalvm.argo.graalvisor.utils.IsolateUtils;
import org.graalvm.nativeimage.CurrentIsolate;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;
import org.graalvm.nativeimage.ObjectHandle;
import org.graalvm.nativeimage.c.function.CEntryPoint;

public class PolyContextSandboxHandle extends SandboxHandle {

    private final PolyContextSandboxProvider csProvider;

    private final IsolateThread isolateThread;

    public PolyContextSandboxHandle(PolyContextSandboxProvider csProvider, IsolateThread isolateThread) {
        this.csProvider = csProvider;
        this.isolateThread = isolateThread;
    }

    @CEntryPoint
    public static ObjectHandle invokeFunction(@CEntryPoint.IsolateThreadContext IsolateThread processContext, IsolateThread defaultContext,
                    ObjectHandle functionHandle, ObjectHandle argumentHandle) throws Exception {
        String functionName = retrieveString(functionHandle);
        String argumentString = retrieveString(argumentHandle);
        String resultString;
        PolyglotFunction function = RuntimeProxy.FTABLE.get(functionName);

        if (function == null || !(function instanceof TruffleFunction)) {
            resultString = String.format("{'Error': 'Function %s not registered or not truffle function!'}", functionName);
        } else {
            TruffleFunction tf = (TruffleFunction) function;
            resultString = RuntimeProxy.LANGUAGE_ENGINE.invoke(tf.getLanguage().toString(), tf.getSource(), tf.getEntryPoint(), argumentString);
        }

        return IsolateUtils.copyString(defaultContext, resultString);
    }

    @Override
    public String invokeSandbox(String jsonArguments) throws Exception {
        PolyglotFunction function = csProvider.getFunction();
        ObjectHandle nameHandle = copyString(isolateThread, function.getName());
        ObjectHandle argsHandle = copyString(isolateThread, jsonArguments);
        return retrieveString(invokeFunction(isolateThread, CurrentIsolate.getCurrentThread(), nameHandle, argsHandle));
    }

    @Override
    public String toString() {
        return Long.toString(Isolates.getIsolate(isolateThread).rawValue());
    }

}
