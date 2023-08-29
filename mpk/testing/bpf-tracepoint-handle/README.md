# BPF demos

- hello

This demo is a small BPF application that intercepts the `write` syscall. It uses `bpf_printk()` BPF helper for debugging. To see its output, read `/sys/kernel/debug/tracing/trace_pipe` file as a super-user. 

## Running the demos

Compile the examples:
```sh
$ make
```

Then, run the program with super-user privileges:
```sh
$ sudo ./build/bin/<demo>
```
