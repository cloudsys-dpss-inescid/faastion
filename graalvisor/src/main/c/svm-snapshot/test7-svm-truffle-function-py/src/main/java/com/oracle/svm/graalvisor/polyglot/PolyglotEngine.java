package com.oracle.svm.graalvisor.polyglot;

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
    private final Map<Value, AtomicBoolean> contexts = new HashMap<>();

    /**
     * Each thread has saves the context it used last. This is a way to reduce races for contexts;
     */
    private final ThreadLocal<Value> contextHint = new ThreadLocal<>();

    /**
     * Each sandbox has a corresponding truffle engine that should be used for the invocation.
     */
    private final Engine engine = COMPILATION ? Engine.create() : null;

    public Value newContext(String language, String source, String entrypoint) {
        Map<String, String> options = new HashMap<>();
        String javaHome = System.getenv("JAVA_HOME");
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
            // Necessary to allow python imports.
            options.put("python.ForceImportSite", "true");
            // Loading the virtual env with installed packages
            options.put("python.Executable", javaHome + "/graalvisor-python-venv/bin/python");
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
        contextHint.set(value);
        contexts.put(value, new AtomicBoolean(false));
        return value;
    }

    private void releaseContext() {
        contexts.get(contextHint.get()).set(true);
    }

    public String invoke(String language, String source, String entrypoint, String arguments) {
        String resultString = new String();

        try {
            resultString = acquireContext(language, source, entrypoint).execute(arguments).toString();
        } catch (PolyglotException pe) {
            if (pe.isSyntaxError()) {
                 resultString = String.format("Error happens during parsing the polyglot function at line %s: %s", pe.getSourceLocation(), pe.getMessage());
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
