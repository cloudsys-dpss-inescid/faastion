#!/usr/bin/python3

import os
import sys
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import numpy as np

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <experiment_dir>")

EXPERIMENTS_DIR = sys.argv[1]

REQUESTS = [1, 8, 14, 20, 32, 48, 64] # order of request concurrency
BENCHMARKS = ["gv_bfs", "gv_compression", "gv_mst", "gv_pagerank", "gv_dna", "gv_dynamic_html", "gv_uploader"]
BASELINES = ["faastion", "hydra", "hydra_si"]
COLORS = {
    "faastion":mcolors.TABLEAU_COLORS['tab:blue'],
    "hydra":mcolors.TABLEAU_COLORS['tab:green'],
    "hydra_si":mcolors.TABLEAU_COLORS['tab:purple'],
}
MARKERS = {
    "faastion":"o",
    "hydra":"^",
    "hydra_si":"x",
}

benchmarks = {}

for benchmark in BENCHMARKS:
    benchmark_path = os.path.join(EXPERIMENTS_DIR, benchmark)
    if not os.path.isdir(benchmark_path):
        continue

    benchmarks[benchmark] = {}

    for baseline in BASELINES:
        baseline_path = os.path.join(benchmark_path, baseline)
        if not os.path.isdir(baseline_path):
            continue

        latency_file = os.path.join(baseline_path, "tput.txt")
        memory_file = os.path.join(baseline_path, "mem.txt")

        if not (os.path.exists(latency_file) and os.path.exists(memory_file)):
            continue

        with open(latency_file) as f:
            latencies = [float(line.strip()) for line in f.read().split()][:len(REQUESTS)]

        with open(memory_file) as f:
            memories = [float(line.strip()) for line in f.read().split()][:len(REQUESTS)]

        benchmarks[benchmark][baseline] = {
            "latency": latencies,
            "memory": memories
        }

print_debug_msg = False
def debug(msg):
    if print_debug_msg:
        print(msg)

def create_subplot(benchmark, ax, data):
    baselines = benchmarks[benchmark].keys()
    workload_size = len(benchmarks[benchmark][list(benchmarks[benchmark].keys())[0]][data])
    debug(workload_size)
    x = np.arange(1, workload_size+1)

    debug(x)

    for baseline in baselines:
        debug(baseline)
        debug(benchmarks[benchmark][baseline][data])
        ax.plot(x, benchmarks[benchmark][baseline][data], label=baseline, color=COLORS[baseline], marker=MARKERS[baseline])

    ax.set_title(benchmark)
    ax.set_xticks(x)
    ax.set_xticklabels(REQUESTS if benchmark != "gv_classify" and benchmark != "gv_thumbnail" else [1, 8, 14])

def create_plot():
    ncols = len(BENCHMARKS)
    nrows = 2
    fig, axes = plt.subplots(nrows, ncols, figsize=(5*ncols, 4*nrows))
    axes = axes.flatten()

    ax_count = 0
    axes[ax_count].set_ylabel("Throughput (Req/sec)")
    for benchmark in BENCHMARKS:
        create_subplot(benchmark, axes[ax_count], "latency")
        if benchmark == "gv_dynamic_html":
            axes[ax_count].set_yscale("log")
        ax_count += 1

    axes[ax_count].set_ylabel("Memory (MiB)")
    for benchmark in BENCHMARKS:
        create_subplot(benchmark, axes[ax_count], "memory")
        ax_count += 1

    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="lower center", ncol=len(REQUESTS), bbox_to_anchor=(0.5, -0.02), borderaxespad=0.)
    fig.suptitle("Latency/Memory Footprint per Benchmark")
    plt.tight_layout()
    plt.savefig("latency_memory.pdf", bbox_inches="tight")
    plt.savefig("latency_memory.png", bbox_inches="tight")
    plt.close()

create_plot()
