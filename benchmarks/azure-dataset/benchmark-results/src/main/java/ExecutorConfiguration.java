
public class ExecutorConfiguration {

    final int numFunctions;
    final String approach;
    /**
     * If true, then print timestamps instead of issuing requests.
     */
    private final boolean debug;
    private final String lambdaManagerAddress;

    ExecutorConfiguration(int numFunctions, String approach, boolean debug, String lambdaManagerAddress) {
        this.numFunctions = numFunctions;
        this.approach = approach;
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
