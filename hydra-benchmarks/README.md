## Benchmarks

This repository includes a number of benchmarks used to test and evaluate Graalvisor and Argo in general. The repository is split into several directories:

- [azure-dataset](azure-dataset) contains a serverless trace generator and simulator. It uses as a base raw trace the [Azure Function trace](https://github.com/Azure/AzurePublicDataset/blob/master/AzureFunctionsDataset2019.md);
- [data](data) contains a set of files used by the benchmarks and a script to launch an NGINX container to serve those files through HTTP;
- [demos](demos) contains a set of experiments/demos/proof-of-concepts that we use to experiment and benchmark new ideas. Each sub-directory explores one specific idea and evaluates it;
- [plots](plots) contains plotting scripts that we use to visualize the results of the experimental evaluation;
- [results](results) is the place where benchmarking scripts will place the log files that later will be processed by plotting scripts;
- [scripts](scripts) contains a number of scripts that help building and launching benchmarks;
- [scr](src) location of the benchmark source code;
- [virtualization](virtualization) contains a set of experiments to evaluate the memory footprint and startup latency of different virtualization technologies;

The repository currently maintains a number of benchmarks. Benchmarks are implemented in two variants: OpenWhisk (cr) and Graalvisor (gv):

- (graalvisor and OpenWhisk) Java:
    - [gv/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/gv-hello-world); [cr/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/cr-hello-world)
    - [gv/file-hashing](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/gv-file-hashing); [cr/file-hashing](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/cr-file-hashing)
    - [gv/http-request](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/gv-httprequest); [cr/http-request](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/gv-httprequest)
    - [gv/video-processing](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/gv-video-processing); [cr/video-processing](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/gv-video-processing)
    - [gv/image-classification](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/gv-classify); [cr/image-classification](https://github.com/graalvm-argo/benchmarks/tree/main/src/java/cr-classify)

- (graalvisor and OpenWhisk) JavaScript:
    - [gv/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/gv-hello-world); [cr/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/cr-hello-world)
    - [gv/dynamic-html](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/gv-dynamic-html); [cr/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/cr-dynamic-html)
    - [gv/uploader](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/gv-uploader); [cr/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/cr-uploader)
    - [gv/thumbnail](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/gv-thumbnail); [cr/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/javascript/cr-thumbnail)

- (graalvisor and OpenWhisk):
    - [gv/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/gv-hello-world); [cr/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/cr-hello-world)
    - [gv/thumbnail](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/gv-thumbnail); [cr/thumbnail](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/cr-thumbnail)
    - [gv/uploader](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/gv-uploader); [cr/hello-world](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/cr-uploader)
    - [gv/compress](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/gv-compress); [cr/compress](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/cr-compress)
    - [gv/video-processing](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/gv-video-processing); [cr/video-processing](https://github.com/graalvm-argo/benchmarks/tree/main/src/python/gv-video-processing)

### Building Benchmarks

To build a benchmark call

`scripts/build_benchmark.sh <path to benchmark build_script.sh>`

or

`BENCHMARK_BUILD_MODE="container" ./build_benchmark.sh <path to benchmark build_script.sh>`

if you want to build inside the argo builder container.

### Running benchmarks

After building a benchmark, it is possible to run it on Graalvisor or OpenWhisk runtimes:
- running the HelloWorld function on a Graalvisor container: `scripts/benchmark-graalvisor.sh container gv_java_hw test 1`
- running the HelloWorld function on a OpenWhisk container: `scripts/benchmark-cruntime.sh container cr_java_hw test 1`
