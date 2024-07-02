import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;
import org.graalvm.nativeimage.c.function.CEntryPoint;

public class IsolateBenchmark {

    @CEntryPoint
    private static float execute(@CEntryPoint.IsolateThreadContext IsolateThread context, long startTime) {
        long finishTime = System.nanoTime();
        return (finishTime - startTime) / (float)1000000;
    }

    public static void main(String[] args) throws Exception {
        int requests = Integer.parseInt(args[0]);
        long total = 0;
        for (int i = 0; i < requests; i++) {
            long startTime = System.nanoTime();
            IsolateThread it = Isolates.createIsolate(Isolates.CreateIsolateParameters.getDefault());
            System.out.println(execute(it, startTime));
            Isolates.tearDownIsolate(it);
        }
    }
}
