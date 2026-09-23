#!/usr/bin/python3

import os
import sys
import matplotlib
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
BASELINES = ["faastlane", "mini_faastion", "faastion"]
BASELINE_NAMES = ["HFI-only", "Faastion (no subprocess)", "Faastion"]
BENCHMARK_NAMES = ["BFS", "MST", "PageRank", "Zip-Compression", "Thumbnailer", "Image-Recognition", "DNA-Visualization", "Dynamic-HTML", "Uploader"]
COLORS = {
    "faastion":mcolors.TABLEAU_COLORS['tab:blue'],
    "faastlane":mcolors.CSS4_COLORS['royalblue'],
    "mini_faastion":mcolors.CSS4_COLORS['antiquewhite'],
}
MARKERS = {
    "faastion":"..",
    "faastlane":"xx",
    "mini_faastion":"\\",
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
        avg_latency_file = os.path.join(baseline_path, "lat.txt")

        if not (os.path.exists(tput_file) and os.path.exists(avg_latency_file)):
            continue

        with open(tput_file) as f:
            tputs = [float(line.strip()) for line in f.read().split()][-1]

        with open(avg_latency_file) as f:
            avg_latencies = [float(line.strip()) for line in f.read().split()][-1]

        benchmarks[benchmark][baseline] = {
            "tput": tputs,
            "latency": avg_latencies
        }

print_debug_msg = False
def debug(msg):
    if print_debug_msg:
        print(msg)

def create_subplot(ax, metric, lbl):
    ax.set_ylabel(lbl)

    bar_width = 0.2
    spacing_factor = 1.2
    index = np.arange(len(BENCHMARKS)) * spacing_factor

    for i, baseline in enumerate(BASELINES):
        if i == 0:
            continue
        bars = [benchmarks[benchmark][baseline][metric] / benchmarks[benchmark]["faastlane"][metric] for benchmark in BENCHMARKS]
        ax.bar(index + i * bar_width, bars, bar_width, label=baseline, color=COLORS[baseline], edgecolor='black', hatch=MARKERS[baseline], linewidth=0.5)

def create_plot():
    ncols = 1
    nrows = 2
    matplotlib.rcParams.update({'font.size': 16})
    fig, axes = plt.subplots(nrows, ncols, sharex=True, figsize=(10, 7))
    axes = axes.flatten()

    create_subplot(axes[0], "tput", "Norm. Throughput")
    create_subplot(axes[1], "latency", "Norm. Latency")

    bar_width = 0.2
    spacing_factor = 1.2
    index = np.arange(len(BENCHMARKS)) * spacing_factor

    ticks0 = np.array((1.0, 1.5, 2.0))
    axes[0].set_yticks(ticks0)
    axes[0].set_yticklabels(ticks0.astype(float))

    ticks1 = np.array((0.4, 0.7, 1.0))
    axes[1].set_yticks(ticks1)
    axes[1].set_yticklabels(ticks1.astype(float))

    axes[1].set_xticks(index + bar_width*len(BASELINES)/2 - bar_width/2, BENCHMARK_NAMES, rotation=45)

    axes[0].grid(axis='y')
    axes[1].grid(axis='y')
    axes[0].set_axisbelow(True)
    axes[1].set_axisbelow(True)

    # baseline_names = [BASELINE_NAMES[lbl] for lbl in labels]
    # fig.legend(handles, baseline_names, loc="upper center", ncol=4, bbox_to_anchor=(0.5, 1.02), borderaxespad=0.)
    # fig.suptitle(title)

    # ax.set_xticks(index + bar_width*len(BASELINES)/2 - bar_width/2, BENCHMARK_NAMES, rotation=45)
    # ax.grid(axis='y')
    # ax.set_axisbelow(True)

    fig.legend(BASELINE_NAMES[1:], loc='upper right', bbox_to_anchor=(0.975, 0.97))

    plt.tight_layout()

    if not os.path.exists(plots_dir):
        os.makedirs(plots_dir)
    
    pdf_file = os.path.join(plots_dir, "ablation_study.pdf")
    png_file = os.path.join(plots_dir, "ablation_study.png")
    plt.savefig(pdf_file, bbox_inches="tight")
    plt.savefig(png_file, bbox_inches="tight")
    plt.close()

create_plot()