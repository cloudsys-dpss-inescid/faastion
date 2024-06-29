package org.graalvm.argo.graalvisor.sandboxing;

public class NativeSandboxInterface {

    public static native boolean isLazyIsolationSupported();

    public static native boolean isMemIsolationSupported();

    public static native void setupMemIsolation(String functionName);

    public static native void teardownMemIsolation(String functionName);

    public static native void ginit();

    public static native int createNativeProcessSandbox(int[] childPipe, int[] parentPipe, boolean lazyIsolation);

    public static native void createNativeIsolateSandbox(boolean lazyIsolation);

    public static native void createNativeRuntimeSandbox(boolean lazyIsolation);
}
