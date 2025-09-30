package com.hello_world;

import java.util.HashMap;
import java.util.Map;
import org.graalvm.word.UnsignedWord;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

@SuppressWarnings("unused")
public class HelloWorld {

    /* For Hydra invocation. */
    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        output.put("Log", "Hello World");
        output.put("VM Context", System.getProperty("java.vm.name"));
        return output;
    }

    /* For standalone invocations. */
    public static void main(String[] args) {
        HashMap<String, Object> output = new HashMap<>();
        output = main(output);
        System.out.println(output);
    }

    /* For c-API invocations. */
    @CEntryPoint(name = "entrypoint")
    public static void main(IsolateThread thread, CCharPointer fin, CCharPointer fout, UnsignedWord foutLen) {
	String input = CTypeConversion.toJavaString(fin);
        String output = main(new HashMap<>()).toString();
	CTypeConversion.toCString(output, fout, foutLen);
    }
}
