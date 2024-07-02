package org.graalvm.argo.dataset.execution;

public class ExecutorConfiguration {

    final byte[] functionCode;
    final String functionLanguage;
    final String functionEntryPoint;
    final String functionMemory;
    final String functionRuntime;
    final String invocationCollocation;
    final String functionIsolation;
    final String gvSandbox;
    /**
     * If true, then print timestamps instead of issuing requests.
     */
    private final boolean debug;
    private final String lambdaManagerAddress;

    ExecutorConfiguration(byte[] functionCode, String functionLanguage, String functionEntryPoint, String functionMemory, String functionRuntime, String invocationCollocation, String functionIsolation, String gvSandbox, boolean debug, String lambdaManagerAddress) {
        this.functionCode = functionCode;
        this.functionLanguage = functionLanguage;
        this.functionEntryPoint = functionEntryPoint;
        this.functionMemory = functionMemory;
        this.functionRuntime = functionRuntime;
        this.invocationCollocation = invocationCollocation;
        this.functionIsolation = functionIsolation;
        this.gvSandbox = gvSandbox;
        this.debug = debug;
        this.lambdaManagerAddress = lambdaManagerAddress;
    }

    public boolean isDebugMode() {
        return debug;
    }

    public String getLambdaManagerAddress() {
        return lambdaManagerAddress;
    }

}
