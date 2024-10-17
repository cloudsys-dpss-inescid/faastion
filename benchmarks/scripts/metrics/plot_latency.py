import os
import sys
import numpy as np
import matplotlib.cm as cm
import matplotlib.pyplot as plt

concurrency_levels = [1, 2, 4, 8, 16, 32]

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <experiment_dir>")

base_dir = sys.argv[1]
plots_dir = "plots"

benchmarks = ['gv_filehashing' ]

approaches = ['isolate', 'faastion_lpi', 'faastion', 'faastlane', 'process']

cmap = plt.get_cmap('viridis')
colors = [cmap(i / len(approaches)) for i in range(len(approaches))]

def read_latency_data(filepath):
    with open(filepath, 'r') as f:
        latencies = [float(line.strip()[:-2]) for line in f.readlines()]
    # print(latencies)
    return latencies

for benchmark in benchmarks:
    plt.figure()

    bar_width = 0.2
    spacing_factor = 1.5  # Factor to increase space between groups
    index = np.arange(len(concurrency_levels)) * spacing_factor  # Base x locations for bars with spacing

    for i, approach in enumerate(approaches):
        latency_file = os.path.join(base_dir, benchmark, 'results', approach, 'latency', '90p.txt')

        if os.path.exists(latency_file):
            latencies = read_latency_data(latency_file)
            plt.bar(index + i * bar_width, latencies, bar_width, label=approach, color=colors[i])
        else:
            print(f"Missing file: {latency_file}")

    plt.xlabel('Concurrency Level')
    plt.ylabel('Average Latency (ms)')
    plt.title(f'Avergae Latency vs Concurrency - {benchmark}')
    plt.xticks(index + bar_width * (len(approaches) - 1) / 2, concurrency_levels)  # Center x-ticks under bars
    plt.legend()  # Show a legend for the approaches

    output_dir = os.path.join(plots_dir, benchmark)
    output_file = os.path.join(output_dir, 'avg_latency.png')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    plt.savefig(output_file)
    plt.close()

print("Plots generated.")
