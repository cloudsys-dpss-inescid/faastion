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

def read_footprint_data(filepath):
    if not os.path.exists(filepath):
        print(f"Missing file: {filepath}")
    with open(filepath, 'r') as f:
        footprint = float(f.read().strip())
    return footprint

def read_stddev_footprint_data(filepath):
    if not os.path.exists(filepath):
        print(f"Missing file: {filepath}")
    with open(filepath, 'r') as f:
        stddev_footprint = float(f.read().strip())
    return stddev_footprint

bar_width = 0.2
spacing_factor = 1.5  # Factor to increase space between groups
index = np.arange(len(benchmarks)) * spacing_factor  # Base x locations for bars with spacing

for i, approach in enumerate(approaches):
    footprint_files = [os.path.join(base_dir, benchmark, 'results', approach, 'memory', 'footprint.txt') for benchmark in benchmarks]
    stddev_footprint_files = [os.path.join(base_dir, benchmark, 'results', approach, 'memory', 'stddev_footprint.txt') for benchmark in benchmarks]

    footprints = []
    stddev_footprints = []
    for idx in range(len(footprint_files)):
        footprints.append(read_footprint_data(footprint_files[idx]))
        stddev_footprints.append(read_stddev_footprint_data(stddev_footprint_files[idx]))
    plt.bar(index + i * bar_width, footprints, bar_width, label=approach, color=colors[i], yerr=stddev_footprints)

plt.ylabel(f'Max Memory Usage (KB)')
plt.title(f'Max Memory Usage vs Benchmark')
plt.xticks(index + bar_width * (len(approaches) - 1) / 2, benchmarks, rotation=45)  # Center x-ticks under bars
plt.legend()  # Show a legend for the approaches

output_dir = os.path.join(plots_dir, 'summary')
output_file = os.path.join(output_dir, 'footprint.pdf')
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

plt.tight_layout()
plt.savefig(output_file)
plt.close()

print("Memory bar plots generated.")
