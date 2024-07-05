import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.*;
import java.nio.file.Files;
import java.nio.file.Paths;

public class InvocationTraceExecutor {

    private static final int MS_IN_HOUR = 3600000;

    private final NetworkUtils network;
    private final String gvSandbox;

    public static Map<Integer, String> hashMap = Map.of(
        0, "nativehw1",
        1, "filehashing",
        2, "factors",
        3, "httprequest",
        4, "matrixmul",
        5, "sleep"
    );

    public static Map<String, String> fcMap = Map.of(
        "nativehw1",    System.getenv("ARGO_HOME") + "/benchmarks/src/java/gv-native-hw-1/build/libnativehw1.so",
        "factors",      System.getenv("ARGO_HOME") + "/benchmarks/src/java/gv-native-factorization/build/libfactors.so",
        "filehashing",  System.getenv("ARGO_HOME") + "/benchmarks/src/java/gv-file-hashing/build/libfilehashing.so",
        "matrixmul",    System.getenv("ARGO_HOME") + "/benchmarks/src/java/gv-native-matrixmul/build/libmatrixmul.so",
        "httprequest",  System.getenv("ARGO_HOME") + "/benchmarks/src/java/gv-httprequest/build/libhttprequest.so",
        "sleep",        System.getenv("ARGO_HOME") + "/benchmarks/src/java/gv-sleep/build/libsleep.so"
    );

    public static Map<String, String> entryMap = Map.of(
        "nativehw1",    "com.jni.HelloJNI",
        "factors",      "com.jni.Factorization",
        "filehashing",  "com.filehashing.FileHashing",
        "matrixmul",    "com.jni.MatrixMultiplication",
        "httprequest",  "com.httprequest.HttpRequest",
        "sleep",        "com.sleep.Sleep"
    );

    public InvocationTraceExecutor(ExecutorConfiguration config) {
        this.gvSandbox = config.approach.equals("process") ? "process" : "isolate";
        this.network = new NetworkUtils(config);
    }

    // HashOwner,HashFunction,AverageAllocatedMb,AverageDuration,Timestamp
    public void execute(String invocationsFilePath) throws IOException {
        uploadFunctions(invocationsFilePath);
        try (BufferedReader br = new BufferedReader(new FileReader(invocationsFilePath))) {
            String line;
            String[] splitRow;
            br.readLine(); // To skip the header
            int currentTimestamp = 0;
            while ((line = br.readLine()) != null) {
                splitRow = line.split(",");

                int function = Integer.valueOf(splitRow[0]);
                int timestamp = Integer.valueOf(splitRow[1]);

                waitForInvocation(currentTimestamp, timestamp);
                currentTimestamp = timestamp;
                invokeFunction(hashMap.get(function));
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void uploadFunctions(String invocationsFilePath) throws IOException {
        Set<String> uploadedFunctions = new HashSet<>();
        try (BufferedReader br = new BufferedReader(new FileReader(invocationsFilePath))) {
            String line;
            String[] splitRow;
            br.readLine(); // To skip the header
            while ((line = br.readLine()) != null) {
                splitRow = line.split(",");
                int function = Integer.parseInt(splitRow[0]);
                ensureUploaded(uploadedFunctions, hashMap.get(function));
            }
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void ensureUploaded(Set<String> uploadedFunctions, String function) throws IOException {
        if (!uploadedFunctions.contains(function)) {
            uploadFunction(function);
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

    private void uploadFunction(String function) throws IOException {
        String queryParameters = "name=" + function + "&language=java&entryPoint=" + entryMap.get(function) + "&sandbox=" + gvSandbox;
        byte[] content = Files.readAllBytes(Paths.get(fcMap.get(function)));
        network.sendPost("/register?" + queryParameters, "application/octet-stream", content, false);
    }

    private void invokeFunction(String function) {
        byte[] data = ("{\"name\":\"" + function + "\",\"async\":\"false\",\"cached\":\"true\",\"arguments\":\"\"}").getBytes(StandardCharsets.UTF_8);
        network.sendPost("/" + function, "application/json; charset=UTF-8", data, true);
     }
}
