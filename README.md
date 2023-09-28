# Faastion

Faastion is a project designed to bridge the gap between language- and hardware-based isolation, providing scalable and secure serverless runtimes.

## Requirements

Before you get started with Faastion, make sure you have the following prerequisites installed:

- [Musl libc](https://musl.libc.org/)
- Linux version >= 5.10 (Seccomp notifications support)
- CPU with MPK (Memory Protection Keys) support
- [Java](https://www.java.com/en/)
- [Maven](https://maven.apache.org/)

## Getting Started

To begin using Faastion, follow these steps:

### Initialize the Erim submodule

```shell
$ git submodule init
$ git submodule update
```

### Compile Erim
Navigate to the Erim source directory and compile it:

```shell
$ cd erim/src
$ make
```

## Test

You can test the Faastion application at different layers. Simply go to the `faastion/mpk/testing` directory and choose the layer you want to test. Each layer follows the same procedure:

1. Navigate to the desired layer directory.
2. Run the following commands:

```shell
$ make
$ make run
```

Feel free to explore and test the different layers to see how Faastion works.

