package org.graalvm.argo.graalvisor;

import java.io.File;

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


    public static void main(String[] args) throws Exception {
        String lambda_port = System.getenv("lambda_port");
        String lambda_timestamp = System.getenv("lambda_timestamp");
        String lambda_isolation = System.getenv("lambda_isolation"); // `lazy` to enable lazy isolation, `eager` (default) to disable
        String lambda_mem_isolation = System.getenv("lambda_mem_isolation"); // `true` to enable mem isolation, `false` (default) to disable.

        if (lambda_timestamp != null) {
            System.out.println(String.format("Graalvisor boot time: %s ms.", (System.currentTimeMillis() - Long.parseLong(lambda_timestamp))));
        }

        if (lambda_port == null) {
            lambda_port = "8080";
        }

        if (APP_DIR == null) {
            APP_DIR = "/tmp/apps/";
        }

        System.out.println(String.format("Graalvisor listening on port %s.", lambda_port));

        if (lambda_isolation != null && lambda_isolation.equals("lazy")) {
            LAZY_ISOLATION_ENABLED = true;
            if (NativeSandboxInterface.isLazyIsolationSupported()) {
                LAZY_ISOLATION_SUPPORTED = true;
            }
            else {
                System.out.println("Warning: graalvisor was compiled without lazy isolation support.");
            }
        }

        if (lambda_mem_isolation != null && lambda_mem_isolation.equals("true")) {
            MEM_ISOLATION_ENABLED = true;
            if (NativeSandboxInterface.isMemIsolationSupported()) {
                MEM_ISOLATION_SUPPORTED = true;
            }
            else {
                System.out.println("Warning: graalvisor was compiled without memory isolation support.");
            }
        }
        // Create the directory where function code will be placed.
        new File(APP_DIR).mkdirs();

        int port = Integer.parseInt(lambda_port);

        if (System.getProperty("java.vm.name").equals("Substrate VM")) {
            // Initialize our native sandbox interface.
            NativeSandboxInterface.ginit();
           new SubstrateVMProxy(port).start();
        } else {
           new HotSpotProxy(port).start();
        }
    }
}
