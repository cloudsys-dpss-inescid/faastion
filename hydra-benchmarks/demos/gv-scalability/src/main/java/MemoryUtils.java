import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class MemoryUtils {
    private static InputStream executeCommand(String... command) throws InterruptedException, IOException {
        ProcessBuilder pb = new ProcessBuilder(command);
        Process process = pb.start();
        process.waitFor();
        return process.getInputStream();
    }

    private static String readToString(BufferedReader reader) throws IOException {
        StringBuilder builder = new StringBuilder();
        String line;
        try (reader) {
            while ((line = reader.readLine()) != null) {
                builder.append(line);
            }
        }
        return builder.toString();
    }

    public static long getRSSKb(long pid) throws Exception {
        InputStream stream = executeCommand("ps", "--no-headers", "eo", "rss", Long.toString(pid));
        return Long.parseLong(readToString(new BufferedReader(new InputStreamReader(stream))).strip());
    }

    public static long getPSSKb(long pid) throws Exception {
        long pss = 0;
        try(BufferedReader br = new BufferedReader(new FileReader(String.format("/proc/%s/smaps", pid)))) {
            for(String line; (line = br.readLine()) != null; ) {
                if (line.startsWith("Pss:")) {
                    pss += Long.parseLong(line.split("\\s+")[1]);
                }
            }
        }
        return pss;
    }
}
