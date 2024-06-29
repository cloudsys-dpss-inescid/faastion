package org.graalvm.argo.graalvisor.function;

import java.util.Locale;

import org.graalvm.argo.graalvisor.sandboxing.SandboxProvider;

import com.oracle.svm.graalvisor.polyglot.PolyglotLanguage;

public class PolyglotFunction {
    private final String name;
    private final String entryPoint;
    private final PolyglotLanguage language;
    private SandboxProvider sprovider;

    public PolyglotFunction(String name, String entryPoint, String language) {
        this.name = name;
        this.entryPoint = entryPoint;
        this.language = PolyglotLanguage.valueOf(language.toUpperCase(Locale.ROOT));
    }

    public String getName() {
        return name;
    }

    public String getEntryPoint() {
        return entryPoint;
    }

    public PolyglotLanguage getLanguage() {
        return language;
    }

    public void setSandboxProvider(SandboxProvider sprovider) {
        this.sprovider = sprovider;
    }

    public SandboxProvider getSandboxProvider() {
        return this.sprovider;
    }
}
