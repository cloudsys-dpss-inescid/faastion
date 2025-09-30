package org.graalvm.argo.graalvisor.utils;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import com.fasterxml.jackson.jr.ob.JSON;

public class JsonUtils {
    public static final JSON json = JSON.std;

    /**
     * Extract arguments json encoded string into Map.
     *
     * @param jsonString json encoded String
     * @return map that illustrates json
     */
    public static Map<String, Object> jsonToMap(String jsonString) {
        try {
            if (jsonString != null && jsonString.length() > 0) {
                return json.mapFrom(jsonString);
            }
        } catch (IOException e) {
            e.printStackTrace(System.err);
        }
        return new HashMap<>();
    }

    public static String appendTmpDirectoryKey(String jsonString, String tmpDirectory) {
        String trimmed = jsonString.trim();
        if (trimmed.endsWith("}")) {
            String prefix = trimmed.substring(0, trimmed.length() - 1).trim();
            if (prefix.endsWith("{")) {
                jsonString = prefix + "\"tmpDir\": \"" + tmpDirectory + "\"}";
            } else {
                jsonString = prefix + ", \"tmpDir\": \"" + tmpDirectory + "\"}";
            }
        } else if (trimmed.isEmpty()) {
            jsonString = "{\"tmpDir\": \"" + tmpDirectory + "\"}";
        } else {
            System.err.println(String.format("[thread %d] Error: unexpected arguments format: %s", Thread.currentThread().getId(), trimmed));
        }
        return jsonString;
    }
}
