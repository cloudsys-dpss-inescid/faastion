import java.util.HashMap;
import java.util.Map;

public class Sleep {
    public static void main(String[] args) throws Exception {
        if (args.length > 0) {
            Thread.sleep(Integer.parseInt(args[0]));
        } else {
            Thread.sleep(10*1000);
        }
    }

    public static Map<String, Object> main(Map<String, Object> input) throws Exception {
        Map<String, Object> output = new HashMap<>();
        main(new String[] {(String)input.get("millis")});
        return output;
    }
}
