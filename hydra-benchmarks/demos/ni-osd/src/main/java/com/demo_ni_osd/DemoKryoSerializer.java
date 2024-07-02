package com.demo_ni_osd;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import com.esotericsoftware.kryo.Kryo;
import com.esotericsoftware.kryo.io.Input;
import com.esotericsoftware.kryo.io.Output;

public class DemoKryoSerializer implements DemoSerializer {

    private Kryo kryo = new Kryo();
    private InputStream in;
    private OutputStream out;

    public DemoKryoSerializer() {
        for (Class<?> clazz : DemoObjects.class.getDeclaredClasses()) {
            kryo.register(clazz);
        }
        kryo.register(ArrayList.class);
        kryo.register(HashMap.class);
    }

    public String serialize(Object o) {
        try {
            Output output = new Output(new FileOutputStream("/tmp/file.dat"));
            kryo.writeObject(output, o);
            output.close();
            return "";
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public Object deserialize(String json, Class<?> clazz) {
        try {
            Input input = new Input(new FileInputStream("/tmp/file.dat"));
            Object o = kryo.readObject(input, clazz);
            input.close();
            return o;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }
}
