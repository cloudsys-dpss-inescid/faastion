import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;
import org.graalvm.nativeimage.c.function.CEntryPoint;

public class IsolateScalabilityTest {

    @CEntryPoint
    private static float execute(@CEntryPoint.IsolateThreadContext IsolateThread context, long startTime) {
           long finishTime = System.nanoTime();
           return (finishTime - startTime) / (float)1000000;
    }

    public static void main(String[] args) throws Exception {
        int requests = Integer.parseInt(args[0]);
        float total = 0;
        for (int i = 0; i < requests; i++) {
            long startTime = System.nanoTime();
            IsolateThread it = Isolates.createIsolate(Isolates.CreateIsolateParameters.getDefault());
            total += execute(it, startTime);
        }

        System.out.println(String.format("Creating %s threads took %s ms!", requests, (total / requests)));
    }
}
