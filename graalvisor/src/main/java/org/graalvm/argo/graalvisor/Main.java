package org.graalvm.argo.graalvisor;

import java.io.File;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.FileNotFoundException;

import org.graalvm.argo.graalvisor.sandboxing.NativeSandboxInterface;

public abstract class Main {

    /**
     * Location where function code will be placed.
     */
    public static String APP_DIR = System.getenv("app_dir");
    public static boolean LAZY_ISOLATION_ENABLED = false;
    public static boolean LAZY_ISOLATION_SUPPORTED = false;
    public static boolean MEM_ISOLATION_ENABLED = false;
    public static boolean MEM_ISOLATION_SUPPORTED = false;

    public static int ACTIVE_WAIT_CAP;
    public static boolean LPI;

    public static void main(String[] args) throws Exception {
        String lambda_port = System.getenv("lambda_port");
        String lambda_timestamp = System.getenv("lambda_timestamp");
        String app_dir = System.getenv("app_dir");

        // System.out.println("Java native image library path: " + System.getProperty("java.library.path"));

        if (lambda_timestamp != null) {
            System.out.println(String.format("Graalvisor boot time: %s ms.", (System.currentTimeMillis() - Long.parseLong(lambda_timestamp))));
        }

        if (lambda_port == null) {
            lambda_port = "8080";
        }

        if (app_dir == null) {
            app_dir = "/tmp/apps/";
        }

        if (MINIO_URL == null) {
            MINIO_URL = "http://127.0.0.1:9000";
        }

        if (MINIO_USER == null) {
            MINIO_USER = "ROOTNAME";
        }

        if (MINIO_PASSWORD == null) {
            MINIO_PASSWORD = "CHANGEME123";
        }

        System.out.println(String.format("Graalvisor listening on port %s.", lambda_port));

        // Create the directory where function code will be placed.
        new File(app_dir).mkdirs();

        int port = Integer.parseInt(lambda_port);

        String cap = System.getenv("ACTIVE_WAIT_CAP");
        ACTIVE_WAIT_CAP = cap == null ? 8 : Integer.parseInt(cap);

        String enableLPI = System.getenv("LPI");
        LPI = enableLPI == null ? false : enableLPI.equals("true");

        if (System.getProperty("java.vm.name").equals("Substrate VM")) {
            Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    server.stop();
                    NativeSandboxInterface.teardown();
                }
            });

            // Initialize our native sandbox interface.
            NativeSandboxInterface.ginit();

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

            new SubstrateVMProxy(port).start();
        } else {
           new HotSpotProxy(port, app_dir).start();
        }
    }
}
