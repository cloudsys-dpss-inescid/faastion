package org.graalvm.argo.graalvisor.sandboxing;

public class NativeSandboxInterface {

    public static native void initialize();
    public static native void teardown();

    public static native int  createNativeProcessSandbox(int[] childPipe, int[] parentPipe);
    public static native void createNativeIsolateSandbox();
    public static native void createNativeRuntimeSandbox();
    // TODO - createContextSandbox and createContextSnapshotSandbox.

    public static native void teardownNativeProcessSandbox();
    public static native void teardownNativeIsolateSandbox();
    public static native void teardownNativeRuntimeSandbox();

    // Methods to access svm-snapshot module (see svm-snapshot.h).
    public static native String svmInvoke(SnapshotSandboxHandle sandboxHandle, String args);
    public static native void svmDestroy(SnapshotSandboxHandle sandboxHandle, boolean reuseIsolate);
    public static native void svmClone(SnapshotSandboxHandle original, SnapshotSandboxHandle clone, boolean reuseIsolate);
    public static native String svmCheckpoint(
        int svmid,
        SnapshotSandboxHandle sandboxHandle,
        int concurrency,
        int requests,
        String functionArgs,
        String functionPath,
        String metaSnapshotPath,
        String memSnapshotPath);
    public static native void svmRestore(
        int svmid,
        SnapshotSandboxHandle sandboxHandle,
        String functionPath,
        String metaSnapshotPath,
        String memSnapshotPath);
    public static native void svmUnload(SnapshotSandboxHandle sandboxHandle);

    // TODO - define function_abi structs.
    public static native long loadFunction(String path);
    public static native long createSandbox(long fabi);
    public static native long getSandbox(long fabi, long ithread);
    public static native long attachThread(long fabi, long isolate);
    public static native String invokeSandbox(long fabi, long ithread, String args);
    public static native int detachThread(long fabi, long ithread);
    public static native void destroySandbox(long fabi, long ithread);
    public static native int unloadFunction(long fabi);
}
