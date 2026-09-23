package com.hello_world;

import org.graalvm.word.UnsignedWord;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;
import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.Engine;
import org.graalvm.polyglot.HostAccess;
import org.graalvm.polyglot.Source;
import org.graalvm.polyglot.Value;
import java.util.HashMap;
import java.util.Map;

public class HelloWorld {

    /* For Hydra invocation. */
    public static HashMap<String, Object> main(Map<String, Object> input) {
        HashMap<String, Object> output = new HashMap<>();
        Map<String,String> options = new HashMap<>();
	options.put("engine.Compilation", "false");

        try (Engine engine = Engine.newBuilder().allowExperimentalOptions(true).options(options).build()) {
            try (Context context = Context.newBuilder().engine(engine).allowHostAccess(HostAccess.ALL).build()) {
                output.put("Log", context.eval(Source.create("python", "2+2")).toString());
            }
        }

        return output;
    }

    /* For standalone invocations. */
    public static void main(String[] args) {
        HashMap<String, Object> input = new HashMap<>();
        HashMap<String, Object> output = main(input);
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
