package org.graalvm.argo.graalvisor;

import static org.graalvm.argo.graalvisor.Main.APP_DIR;
import static com.oracle.svm.graalvisor.utils.JsonUtils.json;
import static com.oracle.svm.graalvisor.utils.JsonUtils.jsonToMap;
import static org.graalvm.argo.graalvisor.utils.ProxyUtils.errorResponse;
import static org.graalvm.argo.graalvisor.utils.ProxyUtils.extractRequestBody;
import static org.graalvm.argo.graalvisor.utils.ProxyUtils.writeResponse;

import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.net.InetSocketAddress;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.charset.StandardCharsets;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Executors;

import org.graalvm.argo.graalvisor.function.HotSpotFunction;
import org.graalvm.argo.graalvisor.function.NativeFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.argo.graalvisor.function.TruffleFunction;
import org.graalvm.argo.graalvisor.sandboxing.PolyContextSandboxProvider;
import org.graalvm.argo.graalvisor.sandboxing.ContextSandboxProvider;
import org.graalvm.argo.graalvisor.sandboxing.IsolateSandboxProvider;
import org.graalvm.argo.graalvisor.sandboxing.ProcessSandboxProvider;
import org.graalvm.argo.graalvisor.sandboxing.RuntimeSandboxProvider;
import org.graalvm.argo.graalvisor.sandboxing.SandboxProvider;
import org.graalvm.argo.graalvisor.utils.ProxyUtils;

import com.oracle.svm.graalvisor.polyglot.PolyglotEngine;
import com.oracle.svm.graalvisor.polyglot.PolyglotLanguage;
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;

/**
 * The runtime proxy exposes a simple webserver that receives three types of requests:
 * - function registration;
 * - function invocation;
 * - function deregistration;
 */
public abstract class RuntimeProxy {

    interface ProxyHttpHandler extends HttpHandler {

        abstract void handleInternal(HttpExchange exchange) throws IOException;

        @Override
        public default void handle(HttpExchange exchange) throws IOException {
            try {
                handleInternal(exchange);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /**
     * Global reference to the engine that runs truffle functions.
     */
    public static final PolyglotEngine LANGUAGE_ENGINE;

    /**
     *  FunctionTable is used to store registered functions inside default and worker isolates.
     */
    public static final ConcurrentHashMap<String, PolyglotFunction> FTABLE = new ConcurrentHashMap<>();

    /**
     * Simple Http server. It uses a cached thread pool for managing threads.
     */
    protected static HttpServer server;

    public RuntimeProxy(int port) throws IOException {
        server = HttpServer.create(new InetSocketAddress(port), -1);
        server.createContext("/", new InvocationHandler());
        server.createContext("/warmup", new WarmupHandler());
        server.createContext("/register", new RegisterHandler());
        server.createContext("/deregister", new DeregisterHandler());
        server.setExecutor(Executors.newCachedThreadPool());
    }

    static {
        PolyglotEngine engine = null;
        try {
            engine = new PolyglotEngine();
        } catch (Throwable e) {
            System.out.println("Warning: graalvisor compiled with no truffle language support.");
        } finally {
            LANGUAGE_ENGINE = engine;
        }
    }

    protected abstract String invoke(PolyglotFunction functionName, boolean cached, boolean warmup, String arguments) throws Exception;

   private String invokeWrapper(String functionName, boolean cached, boolean warmup, String arguments) throws Exception {
      PolyglotFunction function = FTABLE.get(functionName);
      String res;

      long start = System.nanoTime();
      if (function == null) {
            res = String.format("{'Error': 'Function %s not registered!'}", functionName);
        } else {
            res = invoke(function, cached, warmup, arguments);
        }
      long finish = System.nanoTime();

      Map<String, Object> output = new HashMap<>();
        output.put("result", res);
        output.put("process_time(us)", (finish - start) / 1000);
        output.put("function", functionName);
        return json.asString(output);
   }

    public void start() {
        server.start();
    }

    private class InvocationHandler implements ProxyHttpHandler {

        @Override
        public void handleInternal(HttpExchange t) throws IOException {
            try {
                String jsonBody = ProxyUtils.extractRequestBody(t);
                Map<String, Object> input = jsonToMap(jsonBody);
                String functionName = (String) input.get("name");
                String arguments = (String) input.get("arguments");
                String async = (String)input.get("async");
                boolean cached = input.get("cached") == null ? true : Boolean.parseBoolean((String)input.get("cached"));
                boolean debug = input.get("debug") == null ? false : Boolean.parseBoolean((String)input.get("debug"));

                if (debug) {
                    ProxyUtils.writeResponse(t, 200, "Returned from Graalvisor!");
                } else if (async != null && async.equals("true")) {
                    ProxyUtils.writeResponse(t, 200, "Asynchronous request submitted!");
                    String output = invokeWrapper(functionName, cached, false, arguments);
                    System.out.println(output);
                } else {
                    String output = invokeWrapper(functionName, cached, false, arguments);
                    ProxyUtils.writeResponse(t, 200, output);
                }
            } catch (Exception e) {
                e.printStackTrace(System.err);
                errorResponse(t, "An error has occurred (see logs for details): " + e);
            }
        }
    }

    private class WarmupHandler implements ProxyHttpHandler {

        @Override
        public void handleInternal(HttpExchange t) throws IOException {
            try {
                String jsonBody = ProxyUtils.extractRequestBody(t);
                Map<String, Object> input = jsonToMap(jsonBody);
                String functionName = (String) input.get("name");
                String arguments = (String) input.get("arguments");
                String output = invokeWrapper(functionName, false, true, arguments);
                ProxyUtils.writeResponse(t, 200, output);
            } catch (Exception e) {
                e.printStackTrace(System.err);
                errorResponse(t, "An error has occurred (see logs for details): " + e);
            }
        }
    }

    private static class RegisterHandler implements ProxyHttpHandler {

        private SandboxProvider getDefaultSandboxProvider(PolyglotFunction function) {
            if (function.getLanguage() == PolyglotLanguage.JAVA) {
                return new IsolateSandboxProvider(function);
            } else {
                return new PolyContextSandboxProvider(function);
            }
        }

        private SandboxProvider getSandboxProvider(PolyglotFunction function, String sandboxName) {

            if (sandboxName == null) {
                return getDefaultSandboxProvider(function);
            }

            if (function.getLanguage() == PolyglotLanguage.JAVA) {
                if (sandboxName.equals("context")) {
                    return new ContextSandboxProvider(function);
                } else if (sandboxName.equals("isolate")) {
                    return new IsolateSandboxProvider(function);
                } else if (sandboxName.equals("runtime")) {
                    return new RuntimeSandboxProvider(function);
                } else if (sandboxName.equals("process")) {
                    return new ProcessSandboxProvider(function);
                }
            } else {
                if (sandboxName.equals("context")) {
                    return new PolyContextSandboxProvider(function);
                }
            }

            return null;
        }

        @Override
        public void handleInternal(HttpExchange t) throws IOException {
            String[] params = t.getRequestURI().getQuery().split("&");
            Map<String, String> metaData = new HashMap<>();
            for (String param : params) {
                String[] keyValue = param.split("=");
                metaData.put(keyValue[0], keyValue[1]);
            }
            String functionName = metaData.get("name");
            String codeFileName = APP_DIR + functionName;
            String functionEntryPoint = metaData.get("entryPoint");
            String functionLanguage = metaData.get("language");
            String sandboxName = metaData.get("sandbox");
            boolean lazyIsolation = metaData.containsKey("lazyisolation") && metaData.get("lazyisolation").equals("true");
            boolean memIsolation = metaData.containsKey("memisolation") && metaData.get("memisolation").equals("true");

            if (System.getProperty("java.vm.name").equals("Substrate VM") || !functionLanguage.equalsIgnoreCase("java")) {
                handlePolyglotRegistration(t, functionName, codeFileName, functionEntryPoint, functionLanguage, sandboxName, lazyIsolation, memIsolation);
            } else {
                handleHotSpotRegistration(t, functionName, codeFileName, functionEntryPoint);
            }
        }

        private void handleHotSpotRegistration(HttpExchange t, String functionName, String jarFileName, String functionEntryPoint) throws IOException {
            long start = System.currentTimeMillis();
            // This lock will be acquired at most once since only 1 function can be registered in HotSpot mode.
            synchronized (FTABLE) {
                if (!FTABLE.isEmpty()) {
                    writeResponse(t, 200, String.format("Function has already been registered!"));
                    return;
                }

                try (OutputStream fos = new FileOutputStream(jarFileName); InputStream bis = new BufferedInputStream(t.getRequestBody(), 4096)) {
                    // Write JAR to a file.
                    bis.transferTo(fos);

                    // Use class loader explicitly to load new classes from a JAR dynamically.
                    URLClassLoader loader = new URLClassLoader(new URL[] { Paths.get(jarFileName).toUri().toURL() },
                            this.getClass().getClassLoader());
                    Class<?> cls = Class.forName(functionEntryPoint, true, loader);
                    Method method = cls.getMethod("main", Map.class);
                    HotSpotFunction function = new HotSpotFunction(functionName, functionEntryPoint, PolyglotLanguage.JAVA.toString(), method);
                    FTABLE.put(functionName, function);
                } catch (ReflectiveOperationException e) {
                    e.printStackTrace(System.err);
                    errorResponse(t, "An error has occurred (see logs for details): " + e);
                    return;
                }
            }
            System.out.println(String.format("Function %s registered in %s ms", functionName, (System.currentTimeMillis() - start)));
            writeResponse(t, 200, String.format("Function %s registered!", functionName));
        }

        private void handlePolyglotRegistration(HttpExchange t, String functionName, String soFileName, String functionEntryPoint, String functionLanguage, String sandboxName, boolean lazyIsolation, boolean memIsolation) throws IOException {
            long start = System.currentTimeMillis();
            PolyglotFunction function = null;
            SandboxProvider sprovider = null;
            synchronized (FTABLE) {
                if (FTABLE.containsKey(functionName)) {
                    writeResponse(t, 200, String.format("Function %s has already been registered!", functionName));
                    return;
                }

                if (lazyIsolation) {
                    if (!Main.LAZY_ISOLATION_ENABLED) {
                        System.out.println("Warning: Function " + functionName + ": lazy isolation is disabled.");
                        lazyIsolation = false;
                    }
                    else if (!Main.LAZY_ISOLATION_SUPPORTED) {
                        System.out.println("Warning: Function " + functionName + ": graalvisor was compiled without lazy isolation support.");
                        lazyIsolation = false;
                    }
                }

                if (memIsolation) {
                    if (!Main.MEM_ISOLATION_ENABLED) {
                        System.out.println("Warning: Function " + functionName + ": mem isolation is disabled.");
                        memIsolation = false;
                    }
                    else if (!Main.MEM_ISOLATION_SUPPORTED) {
                        System.out.println("Warning: Function " + functionName + ": graalvisor was compiled without mem isolation support.");
                        memIsolation = false;
                    }
                }


                if (functionLanguage.equalsIgnoreCase("java")) {
                    try (OutputStream fos = new FileOutputStream(soFileName); InputStream bis = new BufferedInputStream(t.getRequestBody(), 4096)) {
                        bis.transferTo(fos);
                    }
                    function = new NativeFunction(functionName, functionEntryPoint, functionLanguage, soFileName, lazyIsolation, memIsolation);
                } else {
                    try (InputStream bis = new BufferedInputStream(t.getRequestBody(), 4096)) {
                        String sourceCode = new String(bis.readAllBytes(), StandardCharsets.UTF_8);
                        function = new TruffleFunction(functionName, functionEntryPoint, functionLanguage, sourceCode);
                    }
                }

                sprovider = getSandboxProvider(function, sandboxName);
                if (sprovider == null) {
                    writeResponse(t, 200, String.format("Failed to register fuction: unknown sandbox %s!", sandboxName));
                    return;
                }

                function.setSandboxProvider(sprovider);
                sprovider.loadProvider();
                FTABLE.put(functionName, function);
            }
            System.out.println(String.format("Function %s registered with %s sandboxing in %s ms", functionName, sprovider.getName(), (System.currentTimeMillis() - start)));
            writeResponse(t, 200, String.format("Function %s registered!", functionName));
        }
    }

    private static class DeregisterHandler implements ProxyHttpHandler {

        @Override
        public void handleInternal(HttpExchange t) throws IOException {
            try {
                String functionName = (String) jsonToMap(extractRequestBody(t)).get("name");
                PolyglotFunction function = FTABLE.get(functionName);
                if (function == null) {
                    errorResponse(t, String.format("Function %s has not been registered before!", functionName));
                } else {
                   function.getSandboxProvider().unloadProvider();
                   FTABLE.remove(functionName);
                }
                writeResponse(t, 200, String.format("Function %s removed!", functionName));
            } catch (Exception e) {
                e.printStackTrace(System.err);
                errorResponse(t, "An error has occurred (see logs for details): " + e);
            }
        }
    }
}
