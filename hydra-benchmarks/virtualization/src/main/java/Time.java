import java.util.HashMap;
import java.util.Map;

public class Time {
    public static void main(String[] args) throws Exception {
        long ftime = System.currentTimeMillis();
        long stime = Long.parseLong(args[0]);
        System.out.println(ftime - stime);
    }

    public static Map<String, Object> main(Map<String, Object> input) throws Exception {
        Map<String, Object> output = new HashMap<>();
        main(new String[] { (String) input.get("stime") });
        return output;
    }
}
