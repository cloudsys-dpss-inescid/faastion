package org.graalvm.argo.graalvisor;

import static com.oracle.svm.graalvisor.utils.JsonUtils.json;
import static com.oracle.svm.graalvisor.utils.JsonUtils.jsonToMap;
import static org.graalvm.argo.graalvisor.utils.ProxyUtils.errorResponse;

import java.io.IOException;
import java.lang.reflect.Method;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Map;
import java.util.concurrent.Executors;

import org.graalvm.argo.graalvisor.function.HotSpotFunction;
import org.graalvm.argo.graalvisor.function.PolyglotFunction;
import org.graalvm.argo.graalvisor.function.TruffleFunction;
import org.graalvm.argo.graalvisor.utils.ProxyUtils;

import com.oracle.svm.graalvisor.utils.JsonUtils;
import com.sun.net.httpserver.HttpExchange;
import com.oracle.svm.graalvisor.polyglot.PolyglotLanguage;

/**
 * Runtime proxy that runs on HotSpot JVM. Right now we only support truffle
 * code in HotSpot but it can be extended by running Graalvisor runtimes
 * through JNI.
 * 
 * The HotSpot proxy serves requests in a sequential manner, i.e., no requests will be
 * executed in parallel.
 */
public class HotSpotProxy extends RuntimeProxy {

    public HotSpotProxy(int port) throws Exception {
        super(port);
        server.createContext("/agentconfig", new RetrieveAgentConfigHandler());
        server.setExecutor(Executors.newSingleThreadExecutor());
    }

    @Override
    public String invoke(PolyglotFunction function, boolean cached, boolean warmup, String arguments) throws Exception {
        if (function.getLanguage() == PolyglotLanguage.JAVA) {
            HotSpotFunction hf = (HotSpotFunction) function;
            Method method = hf.getMethod();
            return json.asString(method.invoke(null, new Object[] { JsonUtils.jsonToMap(arguments) }));
        } else if (function instanceof TruffleFunction){
            TruffleFunction tf = (TruffleFunction) function;
            return RuntimeProxy.LANGUAGE_ENGINE.invoke(tf.getLanguage().toString(), tf.getSource(), tf.getEntryPoint(), arguments);
        } else {
            return String.format("{'Error': 'Function %s not registered or not truffle function!'}", function.getName());
        }
    }

    private class RetrieveAgentConfigHandler implements ProxyHttpHandler {

        Map<String, String> configPaths = Map.of("jni",                "config/jni-config.json",
                                                 "predefined-classes", "config/predefined-classes-config.json",
                                                 "proxy",              "config/proxy-config.json",
                                                 "reflect",            "config/reflect-config.json",
                                                 "resource",           "config/resource-config.json",
                                                 "serialization",      "config/serialization-config.json");

        @Override
        public void handleInternal(HttpExchange t) throws IOException {
            try {
                String jsonBody = ProxyUtils.extractRequestBody(t);
                Map<String, Object> input = jsonToMap(jsonBody);
                String configName = (String) input.get("configName");
                String configPath = configPaths.get(configName);
                if (configPath != null) {
                    String configContent = new String(Files.readAllBytes(Paths.get(configPath)));
                    ProxyUtils.writeResponse(t, 200, configContent);
                } else {
                    errorResponse(t, "No such config: " + configName);
                }
            } catch (Exception e) {
                e.printStackTrace(System.err);
                errorResponse(t, "An error has occurred (see logs for details): " + e);
            }
        }
    }
}
