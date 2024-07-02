package com.demo_ni_osdcomms;

import java.io.InputStream;
import java.io.OutputStream;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;

public class DemoGsonSerializer implements DemoSerializer {

    private Gson gson = new GsonBuilder().create();

    public String serialize(Object o) {
        return gson.toJson(o);
    }

    public Object deserialize(String json, Class<?> clazz) {
        return gson.fromJson(json, clazz);
    }
}
