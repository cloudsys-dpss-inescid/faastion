package org.graalvm.argo.dataset.execution;

import org.graalvm.argo.dataset.InvocationTraceGenerator;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.HashSet;
import java.util.Set;

public class InvocationTraceExecutor {

    private static final int MS_IN_HOUR = 3600000;
    private static final int BYTES_IN_MB = 1048576;
    /**
     * Empirical value used to calculate request-specific memory consumption.
     */
    private static final double MEMORY_COEFFICIENT = 0.05;

    private final ExecutorConfiguration config;


    public InvocationTraceExecutor(ExecutorConfiguration config) {
        this.config = config;
    }

    // HashOwner,HashFunction,AverageAllocatedMb,AverageDuration,Timestamp
    public void execute(String invocationsFilePath) {
        uploadFunctions(invocationsFilePath);
        try (BufferedReader br = new BufferedReader(new FileReader(invocationsFilePath))) {
            String line;
            String[] splitRow;
            br.readLine(); // To skip the header
            int currentTimestamp = 0;
            while ((line = br.readLine()) != null) {
                splitRow = line.split(InvocationTraceGenerator.DELIMITER);
                String owner = splitRow[0];
                String function = splitRow[1];
                int allocatedMemoryMb = Integer.valueOf(splitRow[2]);
                int duration = Integer.valueOf(splitRow[3]);
                int timestamp = Integer.valueOf(splitRow[4]);

                waitForInvocation(currentTimestamp, timestamp);
                currentTimestamp = timestamp;

                invokeFunction(owner, function, allocatedMemoryMb, duration, timestamp);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void uploadFunctions(String invocationsFilePath) {
        Set<String> uploadedFunctions = new HashSet<>();
        try (BufferedReader br = new BufferedReader(new FileReader(invocationsFilePath))) {
            String line;
            String[] splitRow;
            br.readLine(); // To skip the header
            while ((line = br.readLine()) != null) {
                splitRow = line.split(InvocationTraceGenerator.DELIMITER);
                String owner = splitRow[0];
                String function = splitRow[1];
                ensureUploaded(uploadedFunctions, owner, function);
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void ensureUploaded(Set<String> uploadedFunctions, String owner, String function) {
        if (!uploadedFunctions.contains(function)) {
            uploadFunction(owner, function);
            uploadedFunctions.add(function);
        }
    }

    private void waitForInvocation(int currentTimestamp, int invocationTimestamp) {
        int timeToSleep = (invocationTimestamp - currentTimestamp) % MS_IN_HOUR;
        if (timeToSleep != 0) {
            try {
                Thread.sleep(timeToSleep);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private void uploadFunction(String owner, String function) {
        String queryParameters = "username=" + owner + "&function_name=" + function +
                "&function_language=" + config.functionLanguage + "&function_entry_point=" + config.functionEntryPoint +
                "&function_memory=" + config.functionMemory + "&function_runtime=" + config.functionRuntime +
                "&function_isolation=" + config.functionIsolation + "&invocation_collocation=" + config.invocationCollocation;
        if (config.gvSandbox != null) {
            queryParameters = queryParameters + "&gv_sandbox=" + config.gvSandbox;
        }
        if (!config.isDebugMode()) {
            NetworkUtils.sendPost(config.getLambdaManagerAddress(), "/upload_function?" + queryParameters, "application/octet-stream", config.functionCode, false);
        }
    }

    private void invokeFunction(String owner, String function, int allocatedMemoryMb, int duration, int timestamp) {
        int memoryToAllocate = (int) (allocatedMemoryMb * BYTES_IN_MB * MEMORY_COEFFICIENT);
        byte[] data = ("{\"memory\":\"" + memoryToAllocate + "\",\"duration\":\"" + duration + "\"}").getBytes(StandardCharsets.UTF_8);
        if (config.isDebugMode()) {
            System.out.println("Sending request with timestamp: " + timestamp);
        } else {
            NetworkUtils.sendPost(config.getLambdaManagerAddress(), "/" + owner + "/" + function, "application/json; charset=UTF-8", data, true);
        }
    }

}
