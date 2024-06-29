package org.graalvm.argo.graalvisor.sandboxing;

import java.io.BufferedReader;
import java.io.FileDescriptor;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.lang.reflect.Constructor;
import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.StackValue;
import org.graalvm.nativeimage.c.function.CFunction;
import org.graalvm.nativeimage.c.type.CIntPointer;

import com.oracle.svm.graalvisor.api.GraalVisorAPI;

import sun.misc.Signal;
import sun.misc.SignalHandler;

public class ProcessSandboxHandle extends SandboxHandle {

    /**
     * Signal number for sending a terminating child processes.
     */
    private static final int SIGKILL = 9;

    /**
     * Process identifier of the subprocess/child.
     */
    private int childPid;

    /**
     * Used to write payloads (parent to child or child to parent).
     */
    private FileOutputStream sender;

    /**
     * Used to read payloads (parent to child or child to parent).
     */
    private BufferedReader receiver;

    static {
         SignalHandler shandler = new SignalHandler() {
            @Override
            public void handle(Signal s) {
                System.exit(0);
            }
        };
        Signal.handle(new Signal("TERM"), shandler);
        Signal.handle(new Signal("INT"), shandler);
    }

    @CFunction
    public static native int kill(int pid, int sig);

    @CFunction
    public static native int waitpid(int pid, CIntPointer stat_loc, int options);

    private void child(ProcessSandboxProvider rsProvider) {
        NativeFunction function = (NativeFunction) rsProvider.getFunction();
        GraalVisorAPI gvAPI = rsProvider.getGraalvisorAPI();
        IsolateThread ithread = gvAPI.createIsolate();
        String line;
        try {
            while((line = receiver.readLine()) != null) {
                sender.write(String.format("%s\n", gvAPI.invokeFunction(ithread, function.getEntryPoint(), line)).getBytes());
            }
        } catch(Exception e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
        } finally {
            destroyChild(childPid);
        }
    }

    private static FileDescriptor createFileDescriptor(int fd) throws Exception {
        Constructor<FileDescriptor> ctor = FileDescriptor.class.getDeclaredConstructor(Integer.TYPE);
        ctor.setAccessible(true);
        return ctor.newInstance(fd);
    }

    public ProcessSandboxHandle(ProcessSandboxProvider rsProvider) throws Exception {
        int[] childPipe = new int[2];
        int[] parentPipe = new int[2];
        if ((childPid = NativeSandboxInterface.createNativeProcessSandbox(childPipe, parentPipe, ((NativeFunction) rsProvider.getFunction()).hasLazyIsolation())) == 0) {
            childPid = (int) ProcessHandle.current().pid();
            sender = new FileOutputStream(createFileDescriptor(childPipe[1]));
            receiver = new BufferedReader(new FileReader(createFileDescriptor(parentPipe[0])));
            child(rsProvider);
        } else {
            sender = new FileOutputStream(createFileDescriptor(parentPipe[1]));
            receiver = new BufferedReader(new FileReader(createFileDescriptor(childPipe[0])));
        }
    }

    @Override
    public String invokeSandbox(String jsonArguments) throws Exception {
        sender.write(String.format("%s\n", jsonArguments).getBytes());
        return receiver.readLine();
    }

    private void destroyChild(int pid) {
        CIntPointer statusptr = StackValue.get(CIntPointer.class);
        kill(pid, SIGKILL);
        waitpid(pid, statusptr, 0);
        try {
            sender.close();
            receiver.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void destroyHandle() throws IOException {
        this.sender.close();
        this.receiver.close();
        destroyChild(childPid);
    }

    @Override
    public String toString() {
        return Integer.toString(childPid);
    }
}
