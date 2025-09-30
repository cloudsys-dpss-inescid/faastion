package org.graalvm.argo.graalvisor;

import java.io.File;

import org.graalvm.argo.graalvisor.sandboxing.NativeSandboxInterface;

public abstract class Main {
    public static String MINIO_URL = System.getenv("minio-url");
    public static String MINIO_SERVER = "minio-storage";
    public static String MINIO_USER = System.getenv("minio-user");
    public static String MINIO_PASSWORD = System.getenv("minio-password");

    public static void main(String[] args) throws Exception {
        String lambda_port = System.getenv("lambda_port");
        String lambda_timestamp = System.getenv("lambda_timestamp");
        String app_dir = System.getenv("app_dir");

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

        if (System.getProperty("java.vm.name").equals("Substrate VM")) {
            NativeSandboxInterface.initialize();
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
