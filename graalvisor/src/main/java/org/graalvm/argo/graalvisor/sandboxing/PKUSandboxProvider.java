package org.graalvm.argo.graalvisor.sandboxing;

import org.graalvm.argo.graalvisor.RuntimeProxy;

import java.io.IOException;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;

import static org.graalvm.argo.graalvisor.RuntimeProxy.FTABLE;

import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;

public class PKUSandboxProvider extends SandboxProvider {

    private static int ACTIVE_WAIT_CAP;
    private static boolean LPI;
    private static boolean pkuIsolationEnabled = false;

    public PKUSandboxProvider(PolyglotFunction function) {
        super(function);
    }

    private static void startLPIWatchdog() {
        new Thread(() -> {
            String poll = System.getenv("ACTIVE_WAIT_POLL");
            long millis = poll == null ? 1000 : Long.parseLong(poll);
            while (true) {
                try {
                    Thread.sleep(millis);
                    NativeSandboxInterface.resetActiveWaitingCount(0);
                } catch (InterruptedException e) {
                    continue;
                }
                
            }
        }).start();
    }

    private static void startDumpThread() {
        new Thread(() -> {
            String poll = System.getenv("DOMAIN_USAGE_POLL");
            long millis = poll == null ? 100 : Long.parseLong(poll);
                
            FileOutputStream fos = null;
            try {
                fos = new FileOutputStream("domain_usage.txt");
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                System.exit(-1);
            }

            int domainUsage;
            String line;
            while (true) {
                try {
                    Thread.sleep(millis);
                    domainUsage = NativeSandboxInterface.getDomainUsage();
                    line = String.valueOf(domainUsage) + "\n";
                    fos.write(line.getBytes());
                } catch (InterruptedException e) {
                    continue;
                } catch (IOException e) {
                    continue;
                }
            }
        }).start();
    }

    public static void initialize() {
        String pku = System.getenv("pku_isolation");
        pkuIsolationEnabled = pku == null ? false : pku.equals("on");

        String cap = System.getenv("ACTIVE_WAIT_CAP");
        ACTIVE_WAIT_CAP = cap == null ? 8 : Integer.parseInt(cap);

        String enableLPI = System.getenv("LPI");
        LPI = enableLPI == null ? false : enableLPI.equals("true");

        if (LPI) {
            startLPIWatchdog();
        }

        String reportDomainUsage = System.getenv("DOMAIN_USAGE_DUMP");
        if (reportDomainUsage != null && reportDomainUsage.equals("true")) {
            startDumpThread();
        }
    }

    public static PolyglotFunction useProcess(String functionName) {
        PolyglotFunction function = null;
        if (NativeSandboxInterface.resetActiveWaitingCount(ACTIVE_WAIT_CAP)) {
            function = FTABLE.get(functionName.replaceAll("[\\d.]", "") + "-proc");
        }
        return function;
    }

    @Override
    public void loadProvider() throws IOException {
        if (!pkuIsolationEnabled)  {
            throw new IOException("Cannot load PKUSandboxProvider: PKU isolation is disabled");
        }
        String fpath = ((NativeFunction) getFunction()).getPath();
        this.functionHandle = NativeSandboxInterface.loadFunction(fpath);
    }

    @Override
    public SandboxHandle createSandbox() {
        long ithreadPtr = NativeSandboxInterface.createSandbox(functionHandle);
        return new PKUSandboxHandle(functionHandle, ithreadPtr);
    }

    @Override
    public void destroySandbox(SandboxHandle shandle) throws IOException {
        PKUSandboxHandle pkhandle = (PKUSandboxHandle) shandle;
        NativeSandboxInterface.teardownNativePKUSandbox();
        NativeSandboxInterface.destroySandbox(functionHandle, pkhandle.getIThreadHandle());
        pkhandle.destroyHandle();
    }

    @Override
    public void unloadProvider() throws IOException {
        NativeSandboxInterface.unloadFunction(this.functionHandle);
    }

    @Override
    public String getName() {
        return "pku";
    }

    @Override
    public boolean supportsLPI() {
        return LPI;
    }
}
