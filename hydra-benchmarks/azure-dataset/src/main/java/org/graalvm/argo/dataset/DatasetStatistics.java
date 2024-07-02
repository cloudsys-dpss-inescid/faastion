package org.graalvm.argo.dataset;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

/**
 * This class reads a raw dataset and produces several statistics:
 * - average invocation per second per function
 * - average invocation per second per user
 */
public class DatasetStatistics {

    public static final String DELIMITER = ",";
    public static final Map<String, Float> avgFunInvokes = new HashMap<>();
    public static final Map<String, Float> avgUserInvokes = new HashMap<>();
    public static final Map<String, ArrayList<Integer>> userInvokes = new HashMap<>();

    public static void main(String[] args) {
        String datasetPath = args[0];
        try {
            File file = new File(datasetPath);
            BufferedReader br = new BufferedReader(new FileReader(file));
            String line;
            String[] splitRow;
            br.readLine(); // To skip the header
            while ((line = br.readLine()) != null) {
                splitRow = line.split(DELIMITER);
                String user = splitRow[0];
                String function = splitRow[2];
                int funInvokes = 0;
                ArrayList<Integer> currUserInvokes = userInvokes.get(user);

                if (currUserInvokes == null) {
                    currUserInvokes = new ArrayList<Integer>(Collections.nCopies(1440, 0));
                    userInvokes.put(user, currUserInvokes);
                }

                for (int i = 0; i < 1440; i++) {
                    int invokes = Integer.parseInt(splitRow[4 + i]);
                    funInvokes += invokes;
                    currUserInvokes.set(i, currUserInvokes.get(i) + invokes);
                }
                avgFunInvokes.put(function, ((float)funInvokes)/1440.0f);
            }
            br.close();

            // Calculate average invocations per user per minute.
            for (String user : userInvokes.keySet()) {
                int totalUserInvokes = 0;
                ArrayList<Integer> currUserInvokes = userInvokes.get(user);
                for (int i = 0; i < 1440; i++) {
                    totalUserInvokes += currUserInvokes.get(i);
                }
                avgUserInvokes.put(user, ((float)totalUserInvokes/1440.0f));
            }

            // Print sorted avg function invocations.
            List<Entry<String, Float>> sortedFunctions = new ArrayList<>(avgFunInvokes.entrySet());
            sortedFunctions.sort(Entry.comparingByValue());
            for(Entry<String, Float> entry : sortedFunctions) {
                System.out.println(String.format("Function %s invoked on average %s #/min", entry.getKey(), entry.getValue()));
            }

            // Print sorted avg user invocations.
            List<Entry<String, Float>> sortedUsers = new ArrayList<>(avgUserInvokes.entrySet());
            sortedUsers.sort(Entry.comparingByValue());
            for(Entry<String, Float> entry : sortedUsers) {
                System.out.println(String.format("User %s invoked on average %s #/min", entry.getKey(), entry.getValue()));
            }

        } catch(IOException ioe) {
            ioe.printStackTrace();
        }
    }
}
