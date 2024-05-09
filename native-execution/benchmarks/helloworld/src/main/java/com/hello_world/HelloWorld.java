package com.hello_world;

import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("unused")
public class HelloWorld {

    public static void main(String[] args) {
        Map<String, Object> output = new HashMap<>();
        output.put("Log", "Hello World");
        output.put("VM Context", System.getProperty("java.vm.name"));
    }
}
