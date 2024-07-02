# Plot benchmark results

In order to produce the plots from the results, just make sure that you have these files placed in
the same directory as the `plot_benchmark.py` script:

* {cr|gv|ph}_active_lambdas.txt
* {cr|gv|ph}_avg_latency.txt
* {cr|gv|ph}_footprint.txt
* {cr|gv|ph}_max_latency.txt
* {cr|gv|ph}_open_requests.txt

Run the script just with `python3 plot_benchmark.py` command.

## Notes on generating the benchmark results

If you want to generate the numbers by yourself, you should connect to the baremetal and prepare it
if it was just rebooted. You have to run the following commands with superuser privileges:

```
echo 4194304 > /proc/sys/kernel/threads-max
echo 4194304 > /proc/sys/kernel/pid_max
echo 4194304 > /proc/sys/vm/max_map_count

mount -t tmpfs -o size=50000m tmpfs /home/ubuntu/git/argo/lambda-manager/lambda_logs
mount -t tmpfs -o size=100000m tmpfs /home/ubuntu/git/argo/lambda-manager/codebase

chown -R ubuntu:ubuntu /home/ubuntu/git/argo/lambda-manager/codebase
chown -R ubuntu:ubuntu /home/ubuntu/git/argo/lambda-manager/lambda_logs
```

You can run the experiment with the following command:
 ```
 bash benchmark-lm-load.sh {gv|cr} /home/ubuntu/git/benchmarks/azure-dataset/output/result_d04.csv
 ```

It is strongly recommended to run `run deploy lm` before running the main experiment, ensure that
the lambda manager is booted, and kill the process.

**Important note**: double-check the `function_isolation` parameter in the
`curl ... upload_function` command in the `benchmark-lm-load.sh` script before running the
experiment. If it is set to `true`, then it will be running in the Photons mode. If the flag is
not set, or if it is set to any other value, then it will allow co-location of the different
functions in the same lambda (if you choose the GV runtime).

## Notes on generating the benchmark inputs

If you want to regenerate the input for this benchmark, you can go to the `~/git/benchmarks/azure-dataset`
directory, and use the `run.sh` script there. For example, the following command:

```
bash run.sh d04 781 785 8192
```

will generate the input invocations from 13:00 to 13:05 of day 4 with max memory usage of 8 GB.
