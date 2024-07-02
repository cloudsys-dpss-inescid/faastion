package micronaut.benchmark.shopcart;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.net.ConnectException;
import java.net.HttpURLConnection;
import java.net.URL;

import io.micronaut.runtime.Micronaut;

public class Benchmark {

    public static String getHTML(String urlToRead) throws Exception {
        StringBuilder result = new StringBuilder();
        URL url = new URL(urlToRead);
        HttpURLConnection conn = (HttpURLConnection) url.openConnection();
        conn.setRequestMethod("GET");
        try (BufferedReader reader = new BufferedReader(
            new InputStreamReader(conn.getInputStream()))) {
            for (String line; (line = reader.readLine()) != null; ) {
                result.append(line);
            }
        }
        return result.toString();
    }

    public static void main(String[] args) {
        new Thread(new Runnable() {

            @Override
            public void run() {
                while(true)
                try {
                    System.out.println(getHTML("http://127.0.0.1:8080/0"));
                    System.exit(0);
                } catch (ConnectException e) {
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException ie) {
                        ie.printStackTrace();
                    }
                } catch (Exception e) {
                    System.exit(-1);
                }
            }
        }).start();
        Micronaut.run(Application.class);
    }
}
