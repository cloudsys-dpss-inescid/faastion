#!/usr/bin/python3

import os
import sys
import numpy as np
import matplotlib.cm as cm
import matplotlib.pyplot as plt

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <experiment_dir>")

base_dir = sys.argv[1]
plots_dir = "plots"

benchmarks = [
    'gv_native_hw',
    'gv_hello_world',
    'gv_native_matmul',
    'gv_matrixmul',
    'gv_httprequest',
    'gv_aes_encryption',
    'gv_native_factors',
    'gv_factorization',
    'gv_filehashing']

approaches = ['isolate', 'faastion_lpi', 'faastion', 'process']

cmap = plt.get_cmap('viridis')
colors = [cmap(i / len(approaches)) for i in range(len(approaches))]

def read_throughput_data(filepath):
    if not os.path.exists(filepath):
        print(f"Missing file: {filepath}")
    with open(filepath, 'r') as f:
        throughput = float(f.read().strip())
    return throughput

def read_stddev_throughput_data(filepath):
    if not os.path.exists(filepath):
        print(f"Missing file: {filepath}")
    with open(filepath, 'r') as f:
        stddev_throughput = float(f.read().strip())
    return stddev_throughput

bar_width = 0.2
spacing_factor = 1.5  # Factor to increase space between groups
index = np.arange(len(benchmarks)) * spacing_factor  # Base x locations for bars with spacing

for i, approach in enumerate(approaches):
    throughput_files = [os.path.join(base_dir, benchmark, 'results', approach, 'latency', 'throughput.txt') for benchmark in benchmarks]
    stddev_throughput_files = [os.path.join(base_dir, benchmark, 'results', approach, 'latency', 'stddev_throughput.txt') for benchmark in benchmarks]

    throughputs = []
    stddev_throughputs = []
    for idx in range(len(throughput_files)):
        throughputs.append(read_throughput_data(throughput_files[idx]))
        stddev_throughputs.append(read_stddev_throughput_data(stddev_throughput_files[idx]))
    plt.bar(index + i * bar_width, throughputs, bar_width, label=approach, color=colors[i], yerr=stddev_throughputs)

plt.ylabel(f'Throughput (#/s)')
plt.title(f'Throughput vs Benchmark')
plt.xticks(index + bar_width * (len(approaches) - 1) / 2, benchmarks, rotation=45)  # Center x-ticks under bars
plt.yscale('log')
plt.legend()  # Show a legend for the approaches

output_dir = os.path.join(plots_dir, 'summary')
output_file = os.path.join(output_dir, 'throughput.pdf')
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

plt.tight_layout()
plt.savefig(output_file)
plt.close()

print("Plots generated.")
