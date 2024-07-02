import com.google.gson.JsonObject;
import java.lang.InterruptedException;
import com.google.gson.JsonPrimitive;

public class Main {

    static byte[] buffer;

    public static void memory(int bytes) {
        buffer = new byte[bytes];
        for (int i = 0; i < bytes; i++) {
            buffer[i] = 1;
        }
    }

    public static void sleep(long millis, JsonObject response) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException ie) {
            response.addProperty("output", "InterruptedException");
        }
    }

    public static JsonObject main(JsonObject args) {
        JsonObject response = new JsonObject();
        JsonPrimitive memoryObject = args.getAsJsonPrimitive("memory");
        if (memoryObject != null) {
            memory(memoryObject.getAsInt());
        }
        sleep(args.getAsJsonPrimitive("sleep").getAsLong(), response);
        return response;
    }
}
