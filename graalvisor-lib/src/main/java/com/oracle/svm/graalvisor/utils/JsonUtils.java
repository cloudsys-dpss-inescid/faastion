package com.oracle.svm.graalvisor.utils;

import com.fasterxml.jackson.jr.ob.JSON;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

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
            if (jsonString != null && !jsonString.isEmpty()) {
                return json.mapFrom(jsonString);
            }
        } catch (IOException e) {
            e.printStackTrace(System.err);
        }
        return new HashMap<>();
    }
}
