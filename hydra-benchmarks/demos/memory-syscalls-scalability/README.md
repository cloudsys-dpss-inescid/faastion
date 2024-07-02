# Memory-related syscall scalability demos

These demos aims to benchmark execution times of several memory-related system calls:
- malloc
- free
- mmap
- memcpy

## Running the demos

Each demo is placed in a separate directory with a `run.sh` script that invokes the demo.
```sh
$ cd demo/directory/you/wish/to/execute
$ ./run.sh iter_count memory_chunk_size_override
```

By default, the demos work with memory chunks of 1MB, but that can be overridden when invoking the `run.sh` script.

If you wish to run all demos at once, use the `run_all.sh` script:
```sh
$ ./run_all.sh iter_count
```

## Plotting the results

Each demo places the results in the appropriate `.log` files. The `plot.py` script can be used to discover logs and render plots.
```sh
$ ./plot.py                            # will discover all logs and plot those demos
$ ./plot.py demo_dir1 demo_dir2        # will plot only chosen demos
```
