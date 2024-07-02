import org.graalvm.nativeimage.StackValue;
import org.graalvm.nativeimage.c.function.CFunction;
import org.graalvm.nativeimage.c.type.CIntPointer;

import com.oracle.svm.graalvisor.api.GraalVisorAPI;
import com.oracle.svm.graalvisor.types.GuestIsolateThread;

public class GraalvisorHostIsolateBenchmark {

    @CFunction
    public static native int fork();

    @CFunction
    public static native int waitpid(int pid, CIntPointer stat_loc, int options);

    public static void executeInNewIsolate(GraalVisorAPI gvapi, boolean shouldteardown, String imgpath, String funname, long startTime) throws Exception {
        GuestIsolateThread guestThread = gvapi.createIsolate();
        gvapi.invokeFunction(guestThread, funname, String.format("{ \"time\": %s }" , startTime));
        if (shouldteardown) {
            gvapi.tearDownIsolate(guestThread);
        }
    }

    public static void executeInNewThread(String imgpath, String funname, long startTime) throws Exception {
        Thread t = new Thread(new Runnable() {
            @Override
            public void run() {
                try (GraalVisorAPI gvapi = new GraalVisorAPI(imgpath)) {
                    executeInNewIsolate(gvapi, true, imgpath, funname, startTime);
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        });
        t.start();
        t.join();
    }

    public static void executeInNewProcess(GraalVisorAPI gvapi, String imgpath, String funname, long startTime) throws Exception {
        int pid = fork();
        if (pid == 0) {
            executeInNewIsolate(gvapi, false, imgpath, funname, startTime);
            System.exit(0);
        } else {
            CIntPointer statusptr = StackValue.get(CIntPointer.class);
            waitpid(pid, statusptr, 0);
        }
    }

    public static void main(String[] args) throws Exception {
        boolean shouldfork = Boolean.parseBoolean(args[0]);
        int requests = Integer.parseInt(args[1]);
        String imgpath = args[2];
        String funname = args[3];

        GraalVisorAPI gvapi = shouldfork ? new GraalVisorAPI(imgpath) : null;
        for (int i = 0; i < requests; i++) {
            long startTime = System.nanoTime();
            if (shouldfork) {
                executeInNewProcess(gvapi, imgpath, funname, startTime);
            } else {
                executeInNewThread(imgpath, funname, startTime);
            }
        }
    }
}
