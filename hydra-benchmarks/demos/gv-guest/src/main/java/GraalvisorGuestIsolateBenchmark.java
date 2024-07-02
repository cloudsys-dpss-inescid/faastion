

import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class GraalvisorGuestIsolateBenchmark {

    public static Map<String, Object> main(Map<String, Object> input) {
        System.out.println((System.nanoTime() - (Long)input.get("time")) / (float)1000000);
        return new HashMap<>();
    }

}
