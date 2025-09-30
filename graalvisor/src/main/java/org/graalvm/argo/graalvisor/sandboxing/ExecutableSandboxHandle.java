package org.graalvm.argo.graalvisor.sandboxing;

import org.graalvm.argo.graalvisor.function.PolyglotFunction;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.PosixFilePermission;
import java.util.HashSet;

import static java.lang.Runtime.getRuntime;
import static java.lang.System.currentTimeMillis;
import static java.nio.file.attribute.PosixFilePermission.GROUP_EXECUTE;
import static java.nio.file.attribute.PosixFilePermission.GROUP_READ;
import static java.nio.file.attribute.PosixFilePermission.GROUP_WRITE;
import static java.nio.file.attribute.PosixFilePermission.OWNER_EXECUTE;
import static java.nio.file.attribute.PosixFilePermission.OWNER_READ;
import static java.nio.file.attribute.PosixFilePermission.OWNER_WRITE;
import static java.nio.file.attribute.PosixFilePermission.OTHERS_EXECUTE;
import static java.nio.file.attribute.PosixFilePermission.OTHERS_WRITE;
import static java.nio.file.attribute.PosixFilePermission.OTHERS_READ;
import static java.util.Arrays.asList;
import static org.graalvm.argo.graalvisor.Main.MINIO_PASSWORD;
import static org.graalvm.argo.graalvisor.Main.MINIO_SERVER;
import static org.graalvm.argo.graalvisor.Main.MINIO_URL;
import static org.graalvm.argo.graalvisor.Main.MINIO_USER;

public class ExecutableSandboxHandle extends SandboxHandle {
    public static final String DEFAULT_IPROF_FILE_NAME = "/default.iprof";
    private final ExecutableSandboxProvider pgoProvider;

    public ExecutableSandboxHandle(ExecutableSandboxProvider pgoProvider) {
        this.pgoProvider = pgoProvider;
    }

    @Override
    public String invokeSandbox(String jsonArguments) throws IOException {
        final PolyglotFunction function = pgoProvider.getFunction();
        return executeCommand(function.getName(), jsonArguments);
    }

    private String executeCommand(String function, String parameter) {
        StringBuilder response = new StringBuilder();
        try {
            final String newAppPath = pgoProvider.getAppDir() + function + "-app";
            final Runtime runtime = getRuntime();
            final File functionPath = new File(newAppPath);

            if (functionPath.mkdir()) {
                final Path source = Path.of(pgoProvider.getAppDir() + function);
                final Path newDir = Path.of(newAppPath);
                Files.move(source, newDir.resolve(source.getFileName()));
                makeFunctionExecutable(newDir, source);
            }
            executeFunction(function, parameter, newAppPath, runtime, response);
            final Path source = getProfilePath(newAppPath);
//            final Path source = Path.of("/default.iprof");
            sendProfileFile(function, source, newAppPath);
//            sendIprofToMinIo(function, source, newAppPath);
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        return response.toString();
    }

    private static void executeFunction(final String function, final String parameter, final String newAppPath, final Runtime runtime, final StringBuilder response) throws IOException, InterruptedException {
        final String[] envs = {};
        final String command = "." + newAppPath + "/" + function + " " + parameter;
        Process process = runtime.exec(command, envs);

        BufferedReader br = new BufferedReader(
                new InputStreamReader(process.getInputStream()));

        BufferedReader stdError = new BufferedReader(new
                InputStreamReader(process.getErrorStream()));

        String line;
        while ((line = br.readLine()) != null) {
            System.out.println("line: " + line);
            response.append(line);
        }

        System.out.println("Standard error:");
        while ((line = stdError.readLine()) != null) {
            System.out.println(line);
        }

        process.waitFor();

        System.out.println("exit: " + process.exitValue());
    }

    private void sendProfileFile(final String function, final Path source, final String newAppPath) {
        new Thread(() -> {
            try {
                sendIprofToMinIo(function, source, newAppPath);
            } catch (IOException | InterruptedException e) {
                throw new RuntimeException(e);
            }
        }).start();
    }

    private void sendIprofToMinIo(String function, Path source, String newAppPath) throws IOException, InterruptedException {
        if (Files.exists(source)) {
            final File pgoPath = new File(newAppPath + "/pgo-profiles");
            pgoPath.mkdir();

            final Path newDir = Path.of(pgoPath.getPath());
            final String pgoFileName = "pgo-" + currentTimeMillis() + ".iprof";
            Files.move(source, newDir.resolve(pgoFileName));
            copyIprofToMinio(pgoPath, function, pgoFileName);
        }
    }

    private Path getProfilePath(String newAppPath) {
        if (Files.exists(Path.of(newAppPath + DEFAULT_IPROF_FILE_NAME))) {
            return Path.of(newAppPath + DEFAULT_IPROF_FILE_NAME);
        }
        if (Files.exists(Path.of(DEFAULT_IPROF_FILE_NAME))) {
            return Path.of(DEFAULT_IPROF_FILE_NAME);
        }
        return null;
    }

    private void copyIprofToMinio(File pgoPath, String functionName, String pgoFileName) throws IOException, InterruptedException {
        final Runtime runtime = getRuntime();
            final String[] envs = { "LD_LIBRARY_PATH=/lib:/lib64:/tmp/apps:/usr/local/lib",
                    "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
                    "_=/usr/bin/env" };

            // the command below creates an alias for the MINION_SERVER using the MINIO_URL with user and password,
            // then the mc mb command uses the functionName to create a bucket with the same name in the Minio and ignore if already exists.
            // In the end the mc cp copies all iprof files to Minio
            final String command = String.format("mc alias set %s %s %s %s && mc mb --ignore-existing %s/%s"  +
                    " && mc cp %s/%s %s/%s", MINIO_SERVER, MINIO_URL, MINIO_USER, MINIO_PASSWORD, MINIO_SERVER, functionName, pgoPath, pgoFileName, MINIO_SERVER, functionName);

            final String[] commandMinio = { "/bin/bash", "-c", command };
            Process processMinio = runtime.exec(commandMinio, envs, pgoPath);
            BufferedReader brMinio = new BufferedReader(
                    new InputStreamReader(processMinio.getInputStream()));

            BufferedReader stdErrorMinio = new BufferedReader(new
                    InputStreamReader(processMinio.getErrorStream()));

            String lineMinio;
            while ((lineMinio = brMinio.readLine()) != null) {
                System.out.println("lineMinio: " + lineMinio);
            }

            System.out.println("Standard error Minio:");
            while ((lineMinio = stdErrorMinio.readLine()) != null) {
                System.out.println(lineMinio);
            }

            processMinio.waitFor();
            System.out.println("exit Minio: " + processMinio.exitValue());
    }

    private static void makeFunctionExecutable(Path newDir, Path source) throws IOException {
        HashSet<PosixFilePermission> permissions = new HashSet<>(asList(GROUP_EXECUTE,GROUP_READ, GROUP_WRITE, OWNER_EXECUTE, OWNER_READ, OWNER_WRITE, OTHERS_EXECUTE, OTHERS_WRITE, OTHERS_READ));
        final File file = new File(newDir.toString() + "/" + source.getFileName());
        Files.setPosixFilePermissions(file.toPath(), permissions);
    }

    @Override
    public String toString() {
        return null;
    }

}
