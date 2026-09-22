package org.graalvm.argo.faastion;

import java.io.File;

import org.graalvm.argo.faastion.sandboxing.NativeSandboxInterface;
import org.graalvm.argo.faastion.sandboxing.PKUSandboxProvider;

public abstract class Main {

    public static String MINIO_URL = System.getenv("minio-url");
    public static String MINIO_SERVER = "minio-storage";
    public static String MINIO_USER = System.getenv("minio-user");
    public static String MINIO_PASSWORD = System.getenv("minio-password");

    public static void main(String[] args) throws Exception {
        String lambda_port = System.getenv("lambda_port");
        String lambda_timestamp = System.getenv("lambda_timestamp");
        String app_dir = System.getenv("app_dir");

        // System.out.println("Java native image library path: " + System.getProperty("java.library.path"));

        if (lambda_timestamp != null) {
            System.out.println(String.format("Faastion boot time: %s ms.", (System.currentTimeMillis() - Long.parseLong(lambda_timestamp))));
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

        System.out.println(String.format("Faastion listening on port %s.", lambda_port));

        // Create the directory where function code will be placed.
        new File(app_dir).mkdirs();

        int port = Integer.parseInt(lambda_port);

        if (System.getProperty("java.vm.name").equals("Substrate VM")) {
            // Initialize our native sandbox interface.
            NativeSandboxInterface.initialize();
            PKUSandboxProvider.initialize();

            SubstrateVMProxy server = new SubstrateVMProxy(port, app_dir);

            Runtime.getRuntime().addShutdownHook(new Thread() {
                public void run() {
                    server.stop();
                    NativeSandboxInterface.teardown();
                }
            });

            server.start();
        } else {
           new HotSpotProxy(port, app_dir).start();
        }
    }
}
