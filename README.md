# Faastion

Faastion is a project designed to bridge the gap between language- and hardware-based isolation, providing scalable and secure serverless runtimes.

Faastion leverages Graalvisor, a high-performance serverless platform powered by technology developed by the GraalVM team. By combining the concepts of Native Image, Isolate, and Truffle, Graalvisor colocates function invocations at a massive scale, resulting in reduced latency and memory footprint compared to traditional serverless platforms.

## Supported Platforms 🖥️

Faastion is currently under development and is supported only on Debian distributions. It has only been tested on Ubuntu 22.04.2 LTS.

## Requirements 📋

Before you get started with Faastion, ensure you have the following prerequisites installed:

- Linux version >= 5.10 (for Seccomp notifications support)
- CPU with MPK (Memory Protection Keys) support
- [Java](https://www.java.com/en/)
- [Maven](https://maven.apache.org/)
- [Gradle](https://gradle.org/)

## Build and Deploy 🚀

Faastion can be easily launched locally for testing and development purposes by following these steps:

### Setup 🛠️

1. Run the `setup.sh` script to download and install the necessary dependencies.
2. During the setup, you will be prompted to build Graalvisor. Follow the prompts to complete the build process.

### Benchmarking 📊

In the `benchmarks` directory, you will find three main subdirectories:

- `src`: Contains all available applications.
- `azure-dataset`: Contains an Azure dataset trace generator to simulate real-life workloads. Use the `run.sh` script inside `benchmark-results` to consume a trace and send requests to Graalvisor, generating diverse plots.
- `scripts`: Contains various scripts that use the `wrk` benchmarking tool. Inside the `test` directory, there is a script to run an application once. Follow these steps to benchmark an application:

    1. Select and build the application from the `src` directory using the `build_script.sh`.
    2. Register the application by uncommenting lines 63-68 in `test.sh` for your chosen application.
    3. Open two terminal windows:
        - In the first terminal, execute `start-graalvisor.sh`.
        - In the second terminal, execute `test.sh <workload>`.
    4. Note that Faastion is still evolving, and these scripts are in the early stages of development. We are working to make them easier to use.

### Testing (Work-in-Progress) 🧪

You can test the Faastion application at different layers. Follow these steps:

1. Navigate to the `mpk/testing` directory and choose the layer you want to test.
2. Run the following commands in the terminal:

    ```shell
    $ make
    $ make run
    ```

Explore and test the different layers to understand how Faastion works.
