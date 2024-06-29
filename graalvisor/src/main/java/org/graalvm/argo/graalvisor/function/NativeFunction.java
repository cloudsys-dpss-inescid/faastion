package org.graalvm.argo.graalvisor.function;

public class NativeFunction extends PolyglotFunction {

    private final String path;
    private final boolean lazyIsolation;
    private final boolean memIsolation;

    public NativeFunction(String name, String entryPoint, String language, String path, boolean lazyIsolation, boolean memIsolation) {
        super(name, entryPoint, language);
        this.path = path;
        this.lazyIsolation = lazyIsolation;
        this.memIsolation = memIsolation;
    }

    public String getPath() {
        return this.path;
    }

    public boolean hasLazyIsolation() {
        return lazyIsolation;
    }

    public boolean hasMemIsolation() {
        return memIsolation;
    }

}
