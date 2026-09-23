#!/usr/bin/python3

import os
import sys
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
import numpy as np

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <experiment_dir>")

base_dir = sys.argv[1]
dir_path = os.path.dirname(os.path.realpath(__file__))
plots_dir = os.path.join(dir_path, 'plots')

EXPERIMENTS_DIR = sys.argv[1]

BENCHMARKS = ["gv_bfs", "gv_mst", "gv_pagerank", "gv_compression", "gv_thumbnail", "gv_classify", "gv_dna", "gv_dynamic_html", "gv_uploader"]
BASELINES = ["faastion", "hydra", "knative", "hydra_si"]
BASELINE_NAMES = {"hydra":"Hydra", "knative":"Knative", "faastion":"Faastion", "hydra_si":"OpenWhisk", "faastlane":"MPK-only"}
BENCHMARK_NAMES = ["BFS", "MST", "PageRank", "Zip-Compression", "Thumbnailer", "Image-Recognition", "DNA-Visualization", "Dynamic-HTML", "Uploader"]
COLORS = {
    "faastion":mcolors.TABLEAU_COLORS['tab:blue'],
    "hydra":mcolors.TABLEAU_COLORS['tab:orange'],
    "hydra_si":mcolors.TABLEAU_COLORS['tab:purple'],
    "knative":mcolors.TABLEAU_COLORS['tab:green'],
    "faastlane":mcolors.CSS4_COLORS['navy'],
}
MARKERS = {
    "faastion":"o",
    "hydra":"v",
    "hydra_si":"d",
    "knative":"_",
    "faastlane":"s",
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
        latency_file = os.path.join(baseline_path, "lat.txt")

        if not (os.path.exists(tput_file) and os.path.exists(latency_file)):
            continue

        with open(tput_file) as f:
            tputs = [float(line.strip()) for line in f.read().split()]

        with open(latency_file) as f:
            latencies = [float(line.strip()) for line in f.read().split()]

        benchmarks[benchmark][baseline] = {
            "tput": tputs,
            "latency": latencies
        }

print_debug_msg = False
def debug(msg):
    if print_debug_msg:
        print(msg)

def create_subplot(benchmark, ax, name):
    baselines = benchmarks[benchmark].keys()
    workload_size = len(benchmarks[benchmark][list(benchmarks[benchmark].keys())[0]]['latency'])
    debug(workload_size)
    x = np.arange(1, workload_size+1)

    debug(x)

    for baseline in baselines:
        debug(baseline)
        debug(benchmarks[benchmark][baseline]['latency'])
        if benchmark in ['gv_bfs', 'gv_mst', 'gv_pagerank']:
            ax.plot(benchmarks[benchmark][baseline]['tput'][1:], benchmarks[benchmark][baseline]['latency'][1:], \
            label=baseline, color=COLORS[baseline], marker=MARKERS[baseline], linewidth=2, markeredgecolor='black', markeredgewidth=0.5)
        else:
            ax.plot(benchmarks[benchmark][baseline]['tput'], benchmarks[benchmark][baseline]['latency'], \
            label=baseline, color=COLORS[baseline], marker=MARKERS[baseline], linewidth=2, markeredgecolor='black', markeredgewidth=0.5)
            

    ax.set_title(name)

def create_plot():
    # ncols = len(BENCHMARKS)
    # nrows = 1
    ncols = 3
    nrows = 3
    fig, axes = plt.subplots(nrows, ncols, figsize=(5*ncols, 2.5*nrows))
    axes = axes.flatten()

    ax_count = 0
    for benchmark in BENCHMARKS:
        create_subplot(benchmark, axes[ax_count], BENCHMARK_NAMES[ax_count])
        axes[ax_count].grid(axis='y')
        ax_count += 1

    fig.supylabel("Average Latency (ms)")
    fig.supxlabel("Throughput (Requests/sec)")
    handles, labels = axes[0].get_legend_handles_labels()
    baseline_names = [BASELINE_NAMES[lbl] for lbl in labels]
    fig.legend(handles, baseline_names, loc="upper center", ncol=4, bbox_to_anchor=(0.5, 1.02), borderaxespad=0.)
    plt.tight_layout()
        
    if not os.path.exists(plots_dir):
        os.makedirs(plots_dir)
    
    pdf_file = os.path.join(plots_dir, 'throughput.pdf')
    png_file = os.path.join(plots_dir, 'throughput.png')
    plt.savefig(pdf_file, bbox_inches="tight")
    plt.savefig(png_file, bbox_inches="tight")
    plt.close()

create_plot()