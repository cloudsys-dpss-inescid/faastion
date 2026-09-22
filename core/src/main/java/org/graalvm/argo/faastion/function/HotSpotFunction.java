package org.graalvm.argo.faastion.function;

import java.lang.reflect.Method;
import com.oracle.svm.faastion.polyglot.PolyglotLanguage;

public class HotSpotFunction extends PolyglotFunction {

    private final Method method;

    public HotSpotFunction(String name, String entryPoint, Method method) {
        super(name, entryPoint, PolyglotLanguage.JAVA.toString(), false);
        this.method = method;
    }

    public Method getMethod() {
        return this.method;
    }
}
