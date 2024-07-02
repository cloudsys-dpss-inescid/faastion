import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import java.io.IOException;
import java.net.URI;
import java.net.http.HttpClient;
import java.net.http.HttpRequest;
import java.net.http.HttpResponse;
import java.time.Duration;
import java.io.FileWriter;
import java.time.Instant;


public class NetworkUtils {

    private final String latencyFile;
    private final String executionFile;
    private final String address;

    public NetworkUtils(ExecutorConfiguration config) {
        this.latencyFile = "/tmp/" + config.approach + "/latency.csv";
        this.executionFile = "/tmp/" + config.approach + "/execution.csv";
        this.address = config.getLambdaManagerAddress();
    }

    private final HttpClient HTTP_CLIENT = HttpClient.newBuilder()
            .version(HttpClient.Version.HTTP_1_1)
            .connectTimeout(Duration.ofSeconds(30)).build();

    public void sendPost(String path, String contentType, byte[] content, boolean async) {
        HttpRequest request = HttpRequest.newBuilder(URI.create("http://" + address + path))
                .timeout(Duration.ofMinutes(3))
                .header("Content-Type", contentType)
                .header("accept", "application/json; charset=UTF-8")
                .POST(HttpRequest.BodyPublishers.ofByteArray(content)).build();

        if (async) {
            final long startTime = System.nanoTime();
            final long timestamp = Instant.now().toEpochMilli();
            final String function = path.split("/")[1];
            HTTP_CLIENT.sendAsync(request, HttpResponse.BodyHandlers.ofString())
                    .thenApply(response -> {
                        long endTime = System.nanoTime();
                        long elapsedTime = endTime - startTime;
                        String latency = elapsedTime + "," + timestamp + "\n";
                        writeToFile(latencyFile, latency);

                        JsonObject jsonResponse = new JsonParser().parse(response.body()).getAsJsonObject();
                        String processTime = jsonResponse.get("process time (us)").getAsString();
                        String execution = processTime + "," + timestamp + "\n";
                        writeToFile(executionFile, execution);
                        return response;
                    })
                    .thenAccept(response -> {
                        System.out.println(function + " -> " + response.body());
                    });
        } else {
            try {
                HttpResponse<String> response = HTTP_CLIENT.send(request, HttpResponse.BodyHandlers.ofString());
                System.out.println(response.body());
            } catch (IOException | InterruptedException e) {
                throw new RuntimeException(e);
            }
        }
    }

    private void writeToFile(String outputFile, String data) {
        try (FileWriter writer = new FileWriter(outputFile, true)) { // Append mode
            writer.write(data);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
