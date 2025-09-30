package org.graalvm.argo.graalvisor.sandboxing;

import org.graalvm.argo.graalvisor.utils.FileUtils;

import java.io.IOException;
import java.util.concurrent.atomic.AtomicInteger;

public abstract class SandboxHandle {

    private static final String TMP_DIRECTORY_PREFIX = "/tmp/sandbox-";
    public static final AtomicInteger SANDBOX_ID_COUNTER = new AtomicInteger(0);

    private final int sandboxId;

    public SandboxHandle() {
        this.sandboxId = SANDBOX_ID_COUNTER.getAndIncrement();
    }

    public abstract String invokeSandbox(String jsonArguments) throws IOException;

    public void destroyHandle() throws IOException {
        String directoryName = TMP_DIRECTORY_PREFIX + this.sandboxId;
        FileUtils.deleteDirectory(directoryName);
    }

    @Override
    public abstract String toString();

    /**
     * Initializes a temporary directory for this sandbox and returns a path to it.
     * @return temporary directory path.
     */
    public String initSandboxTmpDirectory() {
        String directoryName = TMP_DIRECTORY_PREFIX + this.sandboxId;
        FileUtils.createDirectory(directoryName);
        return directoryName;
    }
}
