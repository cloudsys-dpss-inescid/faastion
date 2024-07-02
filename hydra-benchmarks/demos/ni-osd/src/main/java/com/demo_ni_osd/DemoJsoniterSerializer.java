package com.demo_ni_osd;

import com.jsoniter.JsonIterator;
import com.jsoniter.output.EncodingMode;
import com.jsoniter.output.JsonStream;
import com.jsoniter.spi.DecodingMode;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

public class DemoJsoniterSerializer implements DemoSerializer {

    public String serialize(Object o) {
        return JsonStream.serialize(o);
    }

    public Object deserialize(String json, Class<?> clazz) {
        return JsonIterator.deserialize(json, clazz);
    }
}
