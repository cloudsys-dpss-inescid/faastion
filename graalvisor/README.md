## Lambda Proxies

Lambda proxy is the entry point for function invocations and resides in each deployed VM instance (Native Image UniKernel or VMM). It receives the function invocation (or necessary registration beforehand), makes executions and responds to Worker Manager.

In general, Lambda-proxy works as a HTTP server that handles function invocation requests from Worker Manager.

---

### System Architecture

The following is the architecture diagram of the different versions of Lambda proxy being deployed:

![](docs/ProxyArchitecture.svg "Lambda-proxy Architecture")

---

### Design Philosophy

There are two design dimensions of the proxy need to be considered:

|     Runtime        |   Languange Engine  |
| ----------- | ----------- |
|  HotSpot | Java Engine |
|  Native Image (Isolate) | Polyglot Engine|

We are using different classes to implement these two design dimensions.

`runtime.RuntimeProxy` deals with different manner of invocation under HotSpot and Native Image Isolate runtime. Each `runtime.RuntimeProxy` possesses a language execution engine `engine.LanguageEngine`, to which the invocation would be delegated.

`engine.PolyglotEngine` is using Truffle as its polyglot runtime. The invocation and caching mechanisms are implemented inside `base.TruffleExecutor`

Two build script examples are also listed under src/scripts. The `languageEngine` would be assigned while building (registered as Feature into ImageHeap) and shared among different isolates while running.

Note that `PolyglotProxy` is pre-built and only deployed in Native Image UniKernel, while JavaProxy would be running in form of *HotSpot*, *HotSpot with Tracing Agent* and *Native Image UniKernel*.

---

### Code Structure

[`base`](src/main/java/org/graalvm/argo/graalvisor/base): Helper classes to wrap function, language(enum), isolate. Isolate Factory that registers isolate into threadLocal. TruffleExecutor.

[`engine`](src/main/java/org/graalvm/argo/graalvisor/engine): Language engines

[`runtime`](src/main/java/org/graalvm/argo/graalvisor/runtime): Runtime proxies: HotSpotProxy and IsolateProxy

[`utils`](src/main/java/org/graalvm/argo/graalvisor/utils): utilties classes
