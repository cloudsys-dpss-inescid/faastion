package org.graalvm.argo.graalvisor.function;

public class NativeFunction extends PolyglotFunction {

    private final String path;

    public NativeFunction(String name, String entryPoint, String language, String path, boolean isExecutable) {
        super(name, entryPoint, language, isExecutable);
        this.path = path;
    }

    public String getPath() {
        return this.path;
    }
}
