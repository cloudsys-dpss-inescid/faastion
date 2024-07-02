package com.demo_ni_osd;

import com.dslplatform.json.*;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

public class DemoDsljsonSerializer implements DemoSerializer {

    private final DslJson<Object> dslJson = new DslJson<>();

    public String serialize(Object o) {
        try {
            ByteArrayOutputStream os = new ByteArrayOutputStream();
            dslJson.serialize(o, os);
            return new String(os.toByteArray());
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public Object deserialize(String json, Class<?> clazz) {
        try {
            ByteArrayInputStream is = new ByteArrayInputStream(json.getBytes());
            return dslJson.deserialize(clazz, is);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }
}
