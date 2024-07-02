package com.demo_polyglot;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.Engine;
import org.graalvm.polyglot.Source;
import org.graalvm.polyglot.Value;

public class DemoPolyglot {

    public static List<Context> contexts = new ArrayList<>();

    private static long getMemoryFootprint_MBs() {
        return (Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory()) / (1024 * 1024);
    }

    private static void runContext(Engine engine, Source source, Map<String, String> options, String invokeArgs) {
        Context context = Context.newBuilder().allowAllAccess(true).options(options).engine(engine).build();
        contexts.add(context);
        context.eval(source);
        Value function = context.eval(source.getLanguage(), "main");
        System.out.println(function.execute(invokeArgs).toString());
    }

    public static void main(String[] args) {
        String javaHome = System.getenv("JAVA_HOME");
        Map<String, String> options = new HashMap<>();
        Engine engine = null;

        if (args.length != 5) {
            System.err.println("Syntax:  DemoPolyglot <share engine?> <cache engine> <iterations> <python|js> <path to source file> <arguments>");
            System.err.println("Example: DemoPolyglot <true|false> 3 python thumbnail.py http://localhost:8000/snap.png");
            System.exit(1);
        }

        boolean shareEngine = Boolean.parseBoolean(args[0]);
        int iterations = Integer.parseInt(args[1]);
        String language = args[2];
        String scriptPath = args[3];
        String invokeArgs = args[4];

        if (javaHome == null) {
            System.err.println("JAVA_HOME not found in the environment. Polyglot functionality significantly limited.");
            System.exit(1);
        }

        if (shareEngine) {
            engine = Engine.create();
        }

        Source source = Source.newBuilder(language, new File(scriptPath)).buildLiteral();

        // Setting up the environment.
        System.setProperty("org.graalvm.language.python.home", javaHome + "/languages/python");
        System.setProperty("org.graalvm.language.llvm.home", javaHome + "/languages/llvm");
        System.setProperty("org.graalvm.language.js.home", javaHome + "/languages/js");
        if (source.getLanguage().equals("python")) {
            options.put("python.ForceImportSite", "true");
            options.put("python.Executable", javaHome + "/graalvisor-python-venv/bin/python");
        }

        // Benchmark context creation (latency and footprint).
        for (int i = 0; i < iterations; i++) {
            long start = System.currentTimeMillis();
            runContext(engine == null ? Engine.create() : engine, source, options, invokeArgs);
            long finish = System.currentTimeMillis();
            System.gc();
            System.out.println(String.format("Request in context %d took = %s ms %s MBs", i, finish - start, getMemoryFootprint_MBs()));
        }

        // Benchmark a single context (latency).
        Value function = contexts.get(0).eval(source.getLanguage(), "main");
        for (int i = 0; i < iterations; i++) {
            System.gc();
            long start = System.currentTimeMillis();
            System.out.println(function.execute(invokeArgs).toString());
            long finish = System.currentTimeMillis();
            System.out.println(String.format("Request %d took = %s ms", i, finish - start));
        }
    }
}