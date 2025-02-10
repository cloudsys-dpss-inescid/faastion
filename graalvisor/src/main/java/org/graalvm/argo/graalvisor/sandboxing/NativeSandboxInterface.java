package org.graalvm.argo.graalvisor.sandboxing;

public class NativeSandboxInterface {
    public static native void ginit();

    public static native int createNativeProcessSandbox(int[] childPipe, int[] parentPipe, boolean lazyIsolation);

    public static native void createNativeIsolateSandbox(boolean lazyIsolation);

    public static native void createNativeRuntimeSandbox(boolean lazyIsolation);

    public static native void createIsolateFunction();

    public static native void destroyIsolateFunction();

    public static native boolean resetActiveWaitingCount(int active_waiting_threshold);

    public static native int getDomainUsage();

    public static native void invokeProcessSandbox(String filename);
}
