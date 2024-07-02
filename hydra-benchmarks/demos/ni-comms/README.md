# ni-comms demo

This demo measures the TCP communication overhead of two VMs running on a single host. The demo can run both in JVM and SVM (with PGO) mode.

## Running the demo

Please run all scripts from the current directory where this README file is located.

Before running the demo, please make sure to set the following environment variables:
```sh
JAVA_HOME=/path/to/java/home/
GRAALVM_HOME=/path/to/ee/graalvm/home/
```

Alternatively, you can put the environment variables into a file named `.env` in this directory, and the scripts will automatically source those environment variables.

This demo uses Gradle as a build tool. To build the demo:
```sh
$ ./build.sh
```

This will generate a native image that you can execute using the `run.sh` script, with optional arguments:
```sh
$ ./run.sh <number_of_warmup_runs> <number_of_runs> <communication_buffer_size> <log_name>
```

The script will execute multiple tests and this takes some time, please edit `run.sh` and comment out the tests you do not wish to run. Running the executable for the first time will create profiles that can be used to rebuild the native image with PGO.

## Processing results

Demo results are stored in the `logs` directory and plots can be made using `plot.py` script. Alternatively, one can export the logs to CSV using `to_csv.py` script. Plots are dumped into the `plots` directory. `save_plots.sh` compresses the `plots` directory using `p7zip` and adds a timestamp - useful for archiving the results.