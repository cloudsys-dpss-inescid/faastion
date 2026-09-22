# Faastion Runtime

Faastion combines Native Image isolates, leveraging Language Based Isolation (LBI) during managed code execution; and hardware-assisted memory isolation (e.g., MPK) during native code execution.

If you build your applications using `build_script.sh`, Faastion automatically generates Java bytecode patches and C wrapper functions to ensure isolation across unmanaged runtimes.

## Usage

Faastion exposes HTTP endpoints to register and invoke functions. The code of the register/invoke handlers is available in [`RuntimeProxy.java`](src/main/java/org/graalvm/argo/faastion/RuntimeProxy.java).

The register endpoint only accepts HTTP query parameters (i.e., no body). The main parameters for the register endpoit are:

- `name` - name of the function;
- `url` - URL to the function code, usually an *.so or a *.zip. Faastion expects function code to be hosted on some web server;
- `entryPoint` - Java entrypoint (fully-qualified class name, including the package name);
- `language` - language of the function (Faastion currently supports `java`);
- `sandbox` - sandboxing type to use for the function (`context`, `isolate`, `pku`, `process`);

The invoke endpoint accepts a JSON body containing the following parameters:

- `name` - JSON string with the name of the function;
- `arguments` - JSON string with the arguments passed to the function, commonly another JSON.


