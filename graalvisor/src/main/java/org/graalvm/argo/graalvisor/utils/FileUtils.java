package org.graalvm.argo.graalvisor.utils;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

public class FileUtils {

    public static boolean createDirectory(String directoryPath) {
        Path path = Paths.get(directoryPath);
        if (Files.notExists(path)) {
            try {
                Files.createDirectories(path);
                return true;
            } catch (IOException e) {
                System.err.println(String.format("[thread %d] Error creating %s directory: %s", Thread.currentThread().getId(), directoryPath, e.getMessage()));
            }
        }
        return false;
    }

    public static boolean deleteDirectory(String directoryPath) {
        File directory = new File(directoryPath);
        File[] allContents = directory.listFiles();
        if (allContents != null) {
            for (File file : allContents) {
                deleteDirectory(file.getPath());
            }
        }
        return directory.delete();
    }
}
