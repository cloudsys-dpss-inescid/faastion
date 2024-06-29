package com.oracle.svm.graalvisor.polyglot;

import java.util.HashMap;
import java.util.Map;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.Engine;
import org.graalvm.polyglot.PolyglotException;
import org.graalvm.polyglot.Value;

public class PolyglotEngine {

    /**
     * Each thread owns it function value, where polyglot functions execute.
     */
    private final ThreadLocal<Value> function = new ThreadLocal<>();

    /**
     * Each sandbox has a corresponding truffle engine that should be used for the invocation.
     */
    private final Engine engine = Engine.create();

    public void init(String language, String source, String entrypoint) {
        Map<String, String> options = new HashMap<>();
        String javaHome = System.getenv("JAVA_HOME");

        if (javaHome == null) {
            System.err.println("JAVA_HOME not found in the environment. Polyglot functionality significantly limited.");
        } else {
            System.setProperty("org.graalvm.language.python.home", javaHome + "/languages/python");
            System.setProperty("org.graalvm.language.llvm.home", javaHome + "/languages/llvm");
            System.setProperty("org.graalvm.language.js.home", javaHome + "/languages/js");
        }

        if (PolyglotLanguage.PYTHON.toString().equals(language)) {
            // Necessary to allow python imports.
            options.put("python.ForceImportSite", "true");
            // Loading the virtual env with installed packages
            options.put("python.Executable", javaHome + "/graalvisor-python-venv/bin/python");
        }

        // Build context.
        Context context = Context.newBuilder().allowAllAccess(true).engine(engine).options(options).build();
        System.out.println(String.format("[thread %s] Creating context %s", Thread.currentThread().getId(), context.toString()));

        // Host access to implement missing language functionalities.
        addBindings(language, context);

        // Evaluate source script to load function into the environment.
        context.eval(language, source);

        // Get function handle from the script.
        function.set(context.eval(language, entrypoint));
    }

    public void addBindings(String language, Context context) {
        context.getBindings(language).putMember("polyHostAccess", new PolyglotHostAccess());
    }

    public String invoke(String language, String source, String entrypoint, String arguments) {
        String resultString = new String();

        if (function.get() == null) {
            init(language, source, entrypoint);
        }

        try {
            resultString = function.get().execute(arguments).toString();
        } catch (PolyglotException pe) {
            if (pe.isSyntaxError()) {
                 resultString = String.format("Error happens during parsing the polyglot function at line %s: %s", pe.getSourceLocation(), pe.getMessage());
            }
        } catch (Exception e) {
            resultString = String.format("Error while invoking polyglot function: %s", e.getMessage());
            System.err.println(resultString);
            e.printStackTrace(System.err);
        }

        return resultString;
    }
}
