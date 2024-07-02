import com.google.gson.JsonObject;
import java.lang.InterruptedException;
import com.google.gson.JsonPrimitive;
import java.security.MessageDigest;
import java.util.Random;

public class Main {

    static byte[] buffer;

    public static String genericStuff(int bytes, int duration) throws Exception {
        buffer = new byte[bytes];
        long start = System.currentTimeMillis();
        Random rand = new Random();
        String result = "";

        while (System.currentTimeMillis() < (start + duration)) {
            // This simulates going over the network to getch some data.
            Thread.sleep(100);
            rand.nextBytes(buffer);
            result = new String(MessageDigest.getInstance("MD5").digest(buffer));
        }
        return result;
    }

    public static JsonObject main(JsonObject args) throws Exception {
        JsonObject response = new JsonObject();
        JsonPrimitive memoryObject = args.getAsJsonPrimitive("memory");
        JsonPrimitive durationObject = args.getAsJsonPrimitive("duration");
	genericStuff(args.getAsJsonPrimitive("memory").getAsInt(),
                     args.getAsJsonPrimitive("duration").getAsInt());
        return response;
    }

    public static void main(String[] args) throws Exception {
        JsonObject payload = new JsonObject();
        // Memory in bytes.
        payload.addProperty("memory", args[0]);
        // Duration in milliseconds.
        payload.addProperty("duration", args[1]);
        main(payload);
    }

}
