# Benchmarking tool

This is a benchmarking tool, designed to measure both the time it takes to change the domain of an existing thread, and the time to change its access rights to a region of memory.

## Options
You can choose one of two options (for now...) for each execution
* domain
* access

## Run the tool

To run the tool execute the following commands:
```shell
$ make
$ ./benchmark <option> <number-of-threads>
```

### Example

```shell
$ make
$ ./benchmark domain 10
```
