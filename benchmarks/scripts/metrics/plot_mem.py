#!/usr/bin/python3

import os
import sys
import numpy as np
import pandas as pd
import matplotlib.cm as cm
import matplotlib.pyplot as plt

concurrency_levels = [1, 2, 4, 8, 16, 32, 48, 64]
memory_files = [str(workload) + '-footprint.csv' for workload in concurrency_levels]

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <experiment_dir>")

base_dir = sys.argv[1]
plots_dir = "plots"

benchmarks = ['gv_native_factors', 'gv_filehashing', 'gv_aes_encryption', 'gv_native_hw', 'gv_hello_world']

approaches = ['isolate', 'faastion_lpi', 'faastion', 'faastlane', 'process']

cmap = plt.get_cmap('viridis')
colors = [cmap(i / len(approaches)) for i in range(len(approaches))]

def read_max_memory(filepath):
    if os.path.exists(filepath):
        df = pd.read_csv(filepath, header=None)
        max_memory = df[0].quantile(0.99)
        return max_memory
    return None

for benchmark in benchmarks:
    memory_data = {approach: [] for approach in approaches}

    for approach in approaches:
        for i, file_name in enumerate(memory_files):
            memory_file = os.path.join(base_dir, benchmark, 'results', approach, 'memory', file_name)
            max_memory = read_max_memory(memory_file)

            if max_memory is not None:
                memory_data[approach].append(max_memory)
            else:
                print(f"Missing file: {memory_file}")
                memory_data[approach].append(0)

    plt.figure(figsize=(10, 6))
    bar_width = 0.2
    spacing_factor = 1.5
    index = np.arange(len(concurrency_levels)) * spacing_factor

    for i, approach in enumerate(approaches):
        plt.bar(index + i * bar_width, memory_data[approach], bar_width, label=approach, color=colors[i])

    plt.xlabel('Concurrency Level')
    plt.ylabel('Max Memory Usage (KB)')
    plt.title(f'Max Memory Usage vs Concurrency - {benchmark}')
    plt.xticks(index + bar_width * 1.5, concurrency_levels)
    plt.legend()
    plt.tight_layout()

    output_dir = os.path.join(plots_dir, benchmark)
    output_file = os.path.join(output_dir, 'max_memory.pdf')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    plt.savefig(output_file)
    plt.close()

print("Memory bar plots generated.")