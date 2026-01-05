#!/usr/bin/python3

import os
import sys
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import numpy as np

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <experiment_dir>")

EXPERIMENTS_DIR = sys.argv[1]

BENCHMARKS = ["gv_bfs", "gv_mst", "gv_pagerank", "gv_compression", "gv_thumbnail", "gv_classify", "gv_dna", "gv_dynamic_html", "gv_uploader"]
BASELINES = ["hydra", "knative", "faastion", "hydra_si"]
BASELINE_NAMES = ["Hydra", "Knative", "Faastion", "OpenWhisk"]
BENCHMARK_NAMES = ["BFS", "MST", "PageRank", "Zip-Compression", "Thumbnailer", "Image-Recognition", "DNA-Visualization", "Dynamic-HTML", "Uploader"]
COLORS = {
    "faastion":mcolors.TABLEAU_COLORS['tab:blue'],
    "hydra":mcolors.TABLEAU_COLORS['tab:orange'],
    "hydra_si":mcolors.TABLEAU_COLORS['tab:purple'],
    "knative":mcolors.TABLEAU_COLORS['tab:green'],
}
MARKERS = {
    "faastion":"\\\\",
    "hydra":"//",
    "hydra_si":"xx",
    "knative":"||",
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

        tput_file = os.path.join(baseline_path, "tput.txt")
        latency_file = os.path.join(baseline_path, "99p.txt")
        memory_file = os.path.join(baseline_path, "mem.txt")

        if not (os.path.exists(tput_file) and os.path.exists(latency_file) and os.path.exists(memory_file)):
            continue

        with open(tput_file) as f:
            tputs = [float(line.strip()) for line in f.read().split()][-1]

        with open(latency_file) as f:
            latencies = [float(line.strip()) for line in f.read().split()][-1]

        with open(memory_file) as f:
            memories = [float(line.strip()) / 1000 for line in f.read().split()][-1]

        benchmarks[benchmark][baseline] = {
            "tput": tputs,
            "99p": latencies,
            "memory": memories
        }

print_debug_msg = False
def debug(msg):
    if print_debug_msg:
        print(msg)

def create_subplot(benchmark, ax):
    baselines = benchmarks[benchmark].keys()
    workload_size = len(benchmarks[benchmark][list(benchmarks[benchmark].keys())[0]]['latency'])
    debug(workload_size)
    x = np.arange(1, workload_size+1)

    debug(x)

    for baseline in baselines:
        debug(baseline)
        debug(benchmarks[benchmark][baseline]['latency'])
        ax.bar(benchmarks[benchmark][baseline]['tput'], benchmarks[benchmark][baseline]['latency'], label=baseline, color=COLORS[baseline], marker=MARKERS[baseline])

    ax.set_title(benchmark)

def create_plot(metric, name, title, lbl):
    fig, ax = plt.subplots()

    ax.set_ylabel(lbl)
    # ax.set_xlabel("Requests/sec")

    bar_width = 0.2
    spacing_factor = 1.2
    index = np.arange(len(BENCHMARKS)) * spacing_factor

    for i, baseline in enumerate(BASELINES):
        bars = [0 if baseline == 'hydra' and benchmark == 'gv_classify' else benchmarks[benchmark][baseline][metric] for benchmark in BENCHMARKS]
        ax.bar(index + i * bar_width, bars, bar_width, label=baseline, color=COLORS[baseline], edgecolor='black', hatch=MARKERS[baseline], linewidth=0.5)

    ax.set_xticks(index + bar_width*len(BASELINES)/2 - bar_width/2, BENCHMARK_NAMES, rotation=45)
    ax.grid(axis='y')
    ax.set_axisbelow(True)

    if metric == 'tput':
        ax.set_yscale('log')
    fig.legend(BASELINE_NAMES, loc='upper left', bbox_to_anchor=(0.09, 0.98))
    # fig.suptitle(title)
    plt.tight_layout()
    plt.savefig(name + ".pdf", bbox_inches="tight")
    plt.savefig(name + ".png", bbox_inches="tight")
    plt.close()

create_plot('tput', 'throughput', 'Throughput per Benchmark', "Throughput (Requests/sec)")
create_plot('99p', 'tail_latency', 'Tail Latency per Benchmark', "99% Tail Latency (ms)")
create_plot('memory', 'memory', 'Memory Footprint per Benchmark', "Memory Footprint (GB)")