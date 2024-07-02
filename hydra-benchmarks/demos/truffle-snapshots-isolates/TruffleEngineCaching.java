import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.Isolates;
import org.graalvm.nativeimage.c.function.CEntryPoint;

import org.graalvm.polyglot.Context;
import org.graalvm.polyglot.Engine;
import org.graalvm.polyglot.HostAccess;
import org.graalvm.polyglot.Source;
import org.graalvm.polyglot.Value;
import java.util.HashMap;
import java.util.Map;

// Options for Auxiliary Engine Caching
// Reference: https://www.graalvm.org/22.1/graalvm-as-a-platform/language-implementation-framework/AuxiliaryEngineCachingEnterprise/#usage

public final class TruffleEngineCaching {

    @CEntryPoint
    private static void executeSum(@CEntryPoint.IsolateThreadContext IsolateThread it) {
        Map<String,String> options = new HashMap<>();
        options.put("engine.Cache", "cacheSum.image");
        options.put("engine.CacheCompile", "executed");
        options.put("engine.TraceCache", "true");
        options.put("engine.TraceCompilation", "true");

        try (Engine engine = Engine.newBuilder().allowExperimentalOptions(true).options(options).build()) {
            try (Context context = Context.newBuilder().engine(engine).allowHostAccess(HostAccess.ALL).build()) {
                System.out.println(context.eval(Source.create("js", "2+2")));
            }
        }
    }

    @CEntryPoint
    private static void executeMul(@CEntryPoint.IsolateThreadContext IsolateThread it) {
        Map<String,String> options = new HashMap<>();
        options.put("engine.Cache", "cacheMul.image");
        options.put("engine.CacheCompile", "executed");
        options.put("engine.TraceCache", "true");
        options.put("engine.TraceCompilation", "true");

        try (Engine engine = Engine.newBuilder().allowExperimentalOptions(true).options(options).build()) {
            try (Context context = Context.newBuilder().engine(engine).allowHostAccess(HostAccess.ALL).build()) {
                System.out.println(context.eval(Source.create("js", "2*2")));
            }
        }
    }

    public static void main(String[] args) {
        IsolateThread it1 = Isolates.createIsolate(Isolates.CreateIsolateParameters.getDefault());
        IsolateThread it2 = Isolates.createIsolate(Isolates.CreateIsolateParameters.getDefault());
        executeSum(it1);
        executeMul(it2);
    }
}
