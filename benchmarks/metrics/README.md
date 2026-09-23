# Replicating the Experiments

This directory contains the scripts to evalute Faastion and generate the corresponding plots.

[Optional] To disable hyperthreading:
```bash
sudo ../disable_hyperthreading.sh
```

Before running the experiments, make sure to start the [webserver](../../resources/host_webserver.sh) and [fileserver](../../resources/host_fileserver.sh) on a different terminal.

To begin the experiments:
```bash
./bench.sh
```

The script uses the docker image to run Faastion and the relevant comparison targets. To simplify deployment, the script leverages Faastion's sandbox types: `isolate` represents Hydra and `context` represents Knative. For OpenWhisk, the script launches multiple docker containers.

You can change the number of requests issued in ab (ApacheBench) by changing the `warmup_req` and `req` fields in [data.json](data.json). 

You can also change what benchmarks/baselines you wish to evaluate by commenting out the values in lines 128-144 in [bench.sh](bench.sh).

> [!NOTE]
> We ran the experiments on a machine with 64 cores, thus invoking at most 64 concurrent requests (1 request per core). Feel free to change these numbers accordinly  (`bench.sh` line 149).

## Generating the Plots

Once the experiments have finished runnig, the results will be placed under `experiments`. You can run [plot_throughput.py](plot_throughput.py) to generate the plot in Figure 8; [plot_memory.py](plot_memory.py) to generate the plot in Figure 9; and [plot_ablation.py](plot_ablation.pt) to generate the plot in Figure 11. When executing these, you need to specify the directory containing the experiments, for example:
```bash
source common/setup_python_venv.sh # ensure matplotlib
python3 plot_throughput.py experiments/<experiment_date>
```
The resulting figures will be placed under the `plots` directory.

## Large Scale Experiment

The Large Scale Experiment (LSE) replicates the experiment in [Hydra][hydra]. You will first need to setup the [hydra-scheduler][hydra-scheduler].

[hydra]: https://dl.acm.org/doi/10.1145/3772052.3772267
[hydra-scheduler]: https://github.com/cloudsys-dpss-inescid/hydra-scheduler/tree/si/faastion
