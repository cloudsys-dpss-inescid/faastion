import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import java.net.URL;
import java.net.URLConnection;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import javax.xml.bind.DatatypeConverter;
import com.google.gson.JsonObject;

public class Main {

    public static byte[] fromInputStream(InputStream is) throws Exception {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream();
        int nRead;
        byte[] data = new byte[16384];

        while ((nRead = is.read(data, 0, data.length)) != -1) {
            buffer.write(data, 0, nRead);
        }

        return buffer.toByteArray();
    }

    public static byte[] downloadBytes(String url) {
        try {
            URLConnection conn = new URL(url).openConnection();
            InputStream is = conn.getInputStream();
            byte[] bytes = fromInputStream(is);
            is.close();
            return bytes;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    public static JsonObject main(JsonObject args) {
        JsonObject response = new JsonObject();
        byte[] bytes = downloadBytes((String)args.getAsJsonPrimitive("url").getAsString());
        response.addProperty("size", bytes.length);
        return response;
    }

    public static void main(String[] args) {
        JsonObject response = new JsonObject();
        response.addProperty("url", "http://127.0.0.1:8000/snap.png");
        System.out.println(main(response));
    }
}
