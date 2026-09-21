# Adding a JNI Benchmark to Graalvisor

This guide provides step-by-step instructions on how to add a new JNI benchmark to Graalvisor, compile it, and execute it. Follow these instructions to ensure a smooth integration of your benchmark into the Graalvisor project.

## Prerequisites

Before you start, make sure you have the following prerequisites installed on your system:

- [Musl libc](https://musl.libc.org/)
- Linux version >= 5.10 with Seccomp notifications support
- CPU with MPK (Memory Protection Keys) support
- [Java](https://www.java.com/en/)
- [Maven](https://maven.apache.org/)

## Setting Up Benchmark Folders

1. Create a folder in the [benchmarks/src/java](.) directory and name it `gv-<benchmark-name>`.

2. Inside the newly created folder, create a `src` directory where the benchmark's code will reside. This should have a structure similar to [gv-native-hw/src](./gv-native-hw/src).

3. Copy the following files into your benchmark folder:
   - [build_script.sh](./gv-native-hw/build_script.sh)
   - [build.gradle](./gv-native-hw/build.gradle)
   - [settings.gradle](./gv-native-hw/settings.gradle)

4. Within the `java` directory (inside the `src/main` directory), place your main Java code. In the `c` directory, put your native library along with the JNI generated (or not) header file.
    >Note: In the [HelloJNI.java](./gv-native-hw/src/main/java/com/jni/HelloJNI.java) file, despite having the native method, it is not being loaded in the `System.LoadLibrary` (to be fixed later). Don't worry; it still works this way.

6. Update the `build.gradle` and `settings.gradle` files according to your benchmark's desired name. Also, do the same for `build_script.sh` and make sure every `$CLASS_PATH` environment variable is set according to your needs. In the `build_native_library` function of `build_script.sh`, update the targets to compile your native library.

## Compilation

To compile your benchmark, execute your `build_script.sh`. During the compilation process, you will encounter three questions, you should respond "y" for the third one.

Additionally, you need to compile [Graalvisor](../../../graalvisor/) by executing the following command inside its directory:

```bash
$ ./build.sh local
```

Type anything other than "y" to the prompted questions.

## Update benchmarks scripts 

To integrate your benchmark, follow these steps to update the relevant scripts:

1. Create a function in the [benchmark.sh](../../scripts/benchmarks.sh) script that is similar to the existing functions (e.g., `gv_java_native_hw`). Duplicate and modify everything within this function according to your benchmark's requirements.

2. Update the newly created function with all the necessary details specific to your benchmark. This includes setting paths, configuring parameters, and any other relevant information.

In addition to the `benchmark.sh` script, you'll also need to make adjustments to the [shared.sh](../../scripts/shared.sh) script. Follow these additional steps:

3. Locate the function called `start_svm` within the [shared.sh](../../scripts/shared.sh) script.

4. Update the `LD_LIBRARY_PATH` variable with the path to your benchmark's build directory to ensure that the necessary libraries are loaded correctly.

5. Don't forget to update the `JNI_DIR` variable with the same path to ensure the JNI components are found and utilized properly in your benchmark.

These updates will help your benchmark script run smoothly within the Graalvisor project.

## Execution

Once you've added your benchmark and updated the `benchmark.sh` script, you can execute your benchmark using the following commands:

### Sequential Invocations

```bash
$ ./benchmark-graalvisor.sh svm <your-function> test 1
```

Example:

```bash
$ ./benchmark-graalvisor.sh svm gv_java_native_hw test 1
```

### Parallel Invocations

```bash
$ ./benchmark-graalvisor.sh svm <your-function> benchmark 1
```

Please make sure to replace <your-function> with the actual name you've assigned to your benchmark's function in the `benchmarks.sh` script.