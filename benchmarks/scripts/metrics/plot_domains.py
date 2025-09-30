#!/usr/bin/python3

import os
import sys
import numpy as np
import matplotlib.cm as cm
import matplotlib.pyplot as plt

concurrency_levels = [1, 2, 4, 8, 16, 32, 48, 64]
domain_files = [str(workload) + '-domains.txt' for workload in concurrency_levels]

if len(sys.argv) < 2:
	sys.exit("Sytanx: " + sys.argv[0] + " <experiment_dir>")

base_dir = sys.argv[1]
plots_dir = "plots"

benchmarks = ['gv_native_factors', 'gv_filehashing', 'gv_aes_encryption', 'gv_native_hw', 'gv_hello_world']

approaches = ['faastion_lpi', 'faastion']

cmap = plt.get_cmap('viridis')
colors = [cmap(i / len(approaches)) for i in range(len(approaches))]

def read_domain_usage(filepath):
    with open(filepath, 'r') as f:
        domains = [int(line.strip()) for line in f.readlines()]
    return domains[-1]

for benchmark in benchmarks:
    domain_data = {approach: [] for approach in approaches}

    for approach in approaches:
        for i, file_name in enumerate(domain_files):
            domain_file = os.path.join(base_dir, benchmark, 'results', approach, 'domain_usage', file_name)
            usage = read_domain_usage(domain_file)

            if usage is not None:
                domain_data[approach].append(usage)
            else:
                print(f"Missing file: {domain_file}")
                domain_data[approach].append(0)
        
        # print(domain_data[approach])

    plt.figure(figsize=(10, 6))
    bar_width = 0.2
    spacing_factor = 1.5
    index = np.arange(len(concurrency_levels)) * spacing_factor

    for i, approach in enumerate(approaches):
        plt.bar(index + i * bar_width, domain_data[approach], bar_width, label=approach, color=colors[i])

    plt.xlabel('Concurrency Level')
    plt.ylabel('Domain Usage')
    plt.title(f'Domain Usage vs Concurrency - {benchmark}')
    plt.xticks(index + bar_width * 1.5, concurrency_levels)
    plt.legend()
    plt.tight_layout()

    output_dir = os.path.join(plots_dir, benchmark)
    output_file = os.path.join(output_dir, 'domain_usage.pdf')
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    plt.savefig(output_file)
    plt.close()

print("Plots generated.")
