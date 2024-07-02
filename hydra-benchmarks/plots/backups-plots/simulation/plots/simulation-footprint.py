#!/usr/bin/python

import results
import matplotlib
import matplotlib.pyplot as plt
import numpy as np


RESULTS_FILE_UNOPTIMIZED = "../results/simulation_d02_unoptimized_keepalive_10min.txt"
RESULTS_FILE_OPTIMIZED = "../results/simulation_d02_10inv_per_1min_keepalive_10min.txt"
width = 1.0


x = results.read_column(RESULTS_FILE_OPTIMIZED, 1)
yRunningFootprint = np.array(results.read_column(RESULTS_FILE_UNOPTIMIZED, 20))        / 1024
yRunningFootprintOptimized = np.array(results.read_column(RESULTS_FILE_OPTIMIZED, 20)) / 1024


first = x[0]
x = [elem - first for elem in x]
# Convert from ms to sec
x = [elem / 1000 for elem in x]

plt.rcParams["figure.figsize"] = (10, 7)
plt.rcParams.update({'font.size': 24})
plt.xlabel("Time (s)")
plt.ylabel("Memory (GB)")

plt.plot(x, yRunningFootprintOptimized, linewidth=3, linestyle="--", label='Optimized')
plt.plot(x, yRunningFootprint,          linewidth=3, label='Unoptimized')

plt.grid()
# plt.yscale('log', base=10)
plt.ylim(ymin=0, ymax=1600)
plt.xlim(xmin=0, xmax=1200)
plt.legend(ncol=5, loc='upper center')
plt.savefig("simulation-footprint.pdf", bbox_inches='tight')
plt.savefig("simulation-footprint.png", bbox_inches='tight')
plt.show()
