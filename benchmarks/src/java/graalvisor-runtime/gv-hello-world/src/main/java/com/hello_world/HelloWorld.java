package com.hello_world;

import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class HelloWorld {

    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        output.put("Log", "Hello World");
        output.put("VM Context", System.getProperty("java.vm.name"));
        return output;
    }

    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }
}
