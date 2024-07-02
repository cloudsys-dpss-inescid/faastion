package com.demo_ni_osd;

import java.io.InputStream;
import java.io.OutputStream;

public interface DemoSerializer {
    public String serialize(Object o);
    public Object deserialize(String s, Class<?> clazz);
}
