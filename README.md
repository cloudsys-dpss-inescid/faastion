# Faastion: Elastic and Scalable Native Library Isolation for High-Density Serverless Platforms

**Faastion** is a project designed to bridge the gap between language- and hardware-based isolation, enabling high-concurrency and high-density serverless platforms. You can find more details in ["Faastion: Elastic and Scalable Native Library Isolation for High-Density Serverless Platforms"](faastion-paper).

Faastion leverages [GraalVM Native Image](native-image) isolates  and Memory Protection Keys (MPK) to colocate multiple functions within the same address space. As a result, Faastion provides invocations at a massive scale, resulting in reduced latency and memory footprint compared to traditional serverless platforms.

## Supported Platforms

Faastion is well-tested on Ubuntu 22.04.4 LTS and Debian 13.0. It relies primarily on Linux kernel version >= 5.10 (for Seccomp notifications) and a CPU with MPK (Memory Protection Keys) support.

## Requirements

We recommend using the provided [Dockerfile](images/faastion/Dockerfile) to build the Docker image of the full Faastion system. The Docker image implements our `libc` patch, providing an easy setup with the required configurations for running Faastion.

Once the image has been built, you can compile and run your own applications within the Faastion environment.

To follow this guide, make sure the following utilities are installed:

- [Docker](https://www.docker.com/)
- [Python](https://www.python.org/) (version 3.10 or later)
- [venv](https://docs.python.org/3/library/venv.html): python's venv module
- [ApacheBench (`ab`)](https://httpd.apache.org/docs/2.4/programs/ab.html): `apt-get install apache2-utils`
- [curl](https://curl.se/): `apt-get install curl`
- [wget](https://www.gnu.org/software/wget/): `apt-get install wget`
- [jq](https://jqlang.org/): `apt-get install jq`

## Setup

The following scripts will build Faastion image and compile all the SeBS benchmarks enumerated in the paper. This step may take a while due to native-image AOT compilation and Javassist bytecode analysis.

```bash
source run/sources.sh
cd images/faastion
./build_container_image.sh
./build_benchmarks.sh
```

## Running

You can run the scripts in the `benchmarks` directory to evaluate the system or you can manually launch Faastion and invoke requests from the command line.

To launch Faastion, run:
```bash
docker run -d --rm -v $ARGO_HOME/core/shared:/faastion/core/shared --network host faastion --enable-lpi &> /dev/null
```

Faastion should be listening on port 8080 and waiting for clients to upload function code. Register a function, e.g.: BFS
```bash
curl -s -X POST "127.0.0.1:8080/register?"\
"entryPoint=com.jni.BFS"\
"&language=java"\
"&name=function_name"\
"&sandbox=pku"\
"&url=http://127.0.0.1:8000/apps/libbfs-plugin.zip"
``` 

> [!NOTE]
> Faastion receives a url to download the function code from, in this case `http://127.0.0.1:8000/apps/libbfs-plugin.zip`. You can use the provided [scripts](resources/host_webserver.sh) to make this avaible.

You should receive confirmation that the function code was successfully uploaded. Now you can start making requests, like so:
```bash
curl -s -X POST localhost:8080 -H 'Content-Type: application/json' --data-binary '{"name":"function_name","async":"false","arguments":"{}"}'
```

## Repository Overview

This project repository contains the source code and benchmarks related to [the paper](faastion-paper) referenced above. The contents of the repository are organized as such:

### Instrumentation
- `native-execution/instrumentation`: performs static analysis, locates all native function calls (regardless of whether or not they are used), creates wrapper functions for native libraries and replaces native function calls with stub calls;
- `native-execution/javassist`: generates bytecode at runtime to characterize benchmarks, such as number of real native switches in BFS throughout total execution, measures % of time in native code;  
- `native-execution/scripts`: uses the previous bytecode generation tools to create a collection of results characterizing the benchmarks (Table 1).

### Faastion
- `core`: platform's code, handling function deployment, scaling, and MPK management;
- `common`: contains API shared between the Faastion runtime and the benchmarks;

### Benchmarks
- `benchmarks/src`: directory containing the source code for the multiple SeBS benchmarks. You can find more details on how to create your own functions in [benchmarks/src/java/SeBS](benchmarks/src/java/SeBS/README.md);
- `benchmarks/metrics`: scripts used to evaluate the system. You can find more details on how to reproduce the experiments in [benchmarks/metrics](benchmarks/metrics/README.md);
- `benchmarks/gc-pressure`: micro-benchmark used to evaluate the GC pressure (e.g., in DNA-Visualization or Dynamic-HTML);
- `resources`: contains useful scripts to initialize a simple http server (for Thumbnailer, Dynamic-HTML) and a flask server (for Uploader).

## Acknowledgments

[native-image]: https://www.graalvm.org/latest/reference-manual/native-image/
