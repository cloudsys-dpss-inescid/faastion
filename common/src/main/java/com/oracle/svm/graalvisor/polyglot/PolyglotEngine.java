package com.oracle.svm.graalvisor.polyglot;

import java.util.concurrent.ConcurrentHashMap;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.Engine;
import org.graalvm.polyglot.PolyglotException;
import org.graalvm.polyglot.Value;

public class PolyglotEngine {

    /**
     * Wether Truffle compilation is enabled. Compilation is very expensive for short-running functions.
     */
    private static final boolean COMPILATION = false;

    /**
     * Map of already available contexts. If the value is true, the context is available.
     */
    private final Map<Value, AtomicBoolean> contexts = new ConcurrentHashMap<>();

    /**
     * Each thread has saves the context it used last. This is a way to reduce races for contexts;
     */
    private final ThreadLocal<Value> contextHint = new ThreadLocal<>();

    /**
     * Each sandbox has a corresponding truffle engine that should be used for the invocation.
     */
    private final Engine engine = COMPILATION ? Engine.create() : null;

    private Value newContext(String language, String source, String entrypoint) {
        Map<String, String> options = new HashMap<>();
        String javaHome = System.getenv("JAVA_HOME");
        String hydraPython = System.getenv("HYDRA_PYTHON");
        Context context = null;

        if (javaHome == null) {
            System.err.println("JAVA_HOME not found in the environment. Polyglot functionality significantly limited.");
        } else {
            System.setProperty("org.graalvm.language.python.home", javaHome + "/languages/python");
            System.setProperty("org.graalvm.language.llvm.home", javaHome + "/languages/llvm");
            System.setProperty("org.graalvm.language.js.home", javaHome + "/languages/js");
        }

        // Adding compilation option.
        if (PolyglotLanguage.PYTHON.toString().equals(language)) {
	    if (hydraPython == null) {
                System.err.println("HYDRA_PYTHON not found in the environment. Polyglot functionality significantly limited.");
	    } else {
                // Necessary to allow python imports.
                options.put("python.ForceImportSite", "true");
                // Loading python from Hydra's GraalVM.
                options.put("python.Executable", hydraPython);
	    }
        }

        // Build context.
        if (COMPILATION) {
            context = Context.newBuilder().allowAllAccess(true).engine(engine).options(options).build();
        } else {
            options.put("engine.Compilation", "false");
            context = Context.newBuilder().allowAllAccess(true).allowExperimentalOptions(true).options(options).build();
        }

        // Host access to implement missing language functionalities.
        addBindings(language, context);

        // Evaluate source script to load function into the environment.
        context.eval(language, source);

        // Return the function handle from the script.
        return context.eval(language, entrypoint);
    }

    public void addBindings(String language, Context context) {
        context.getBindings(language).putMember("polyHostAccess", new PolyglotHostAccess());
    }

    private Value acquireContext(String language, String source, String entrypoint) {
        // Hot path: if the hint is set, try to acquire the hinted context.
        if (contextHint.get() != null && contexts.get(contextHint.get()).compareAndSet(true, false)) {
            return contextHint.get();
        }

        // Warm path: if the hint is not set or is taken, iterate all contexts.
        for(Map.Entry<Value, AtomicBoolean> entry : contexts.entrySet()) {
            if (entry.getValue().compareAndSet(true, false)) {
                contextHint.set(entry.getKey());
                return entry.getKey();
            }
        }

        // Cold path: create a new context.
        long stime = System.currentTimeMillis();
        Value value = newContext(language, source, entrypoint);
        long ftime = System.currentTimeMillis();
        System.out.println(String.format("[thread %s] Creating context %s (took %d ms)",
            Thread.currentThread().getId(), value.toString(), ftime - stime));
        contexts.put(value, new AtomicBoolean(false));
        contextHint.set(value);
        return value;
    }

    private void releaseContext() {
        Value val = contextHint.get();
        if (val == null) {
            return;
        }

        AtomicBoolean abool = contexts.get(val);
        if (abool == null) {
            return;
        }

        abool.set(true);
    }

    public void init(String language, String source, String entrypoint) {
        acquireContext(language, source, entrypoint);
        releaseContext();
    }

    public String invoke(String language, String source, String entrypoint, String arguments) {
        String resultString = new String();

        try {
            resultString = acquireContext(language, source, entrypoint).execute(arguments).toString();
        } catch (PolyglotException pe) {
            if (pe.isSyntaxError()) {
                 resultString = String.format("Error happens during parsing the polyglot function at line %s: %s", pe.getSourceLocation(), pe.getMessage());
            } else {
                 resultString = String.format("Error while loading function at line %s: %s", pe.getSourceLocation(), pe.getMessage());
                System.err.println(resultString);
                pe.printStackTrace(System.err);
            }
        } catch (Exception e) {
            resultString = String.format("Error while invoking polyglot function: %s", e.getMessage());
            System.err.println(resultString);
            e.printStackTrace(System.err);
        } finally {
            releaseContext();
        }

        return resultString;
    }
}
