#!/usr/bin/python

import results
import matplotlib
import matplotlib.pyplot as plt
import numpy as np


RESULTS_FILE = "../results/simulation_d02_10inv_per_1min_keepalive_10min.txt"
width = 1.0


x = results.read_column(RESULTS_FILE, 1)
yColdstarts = np.array(results.read_column(RESULTS_FILE, 6))
yOptimizedColdstarts = np.array(results.read_column(RESULTS_FILE, 8))

yUnoptimizedColdstarts = yColdstarts - yOptimizedColdstarts


first = x[0]
x = [elem - first for elem in x]
# Convert from ms to sec
x = [elem / 1000 for elem in x]

plt.rcParams.update({'font.size': 24})
fig, ax = plt.subplots()

bottom = np.zeros(len(x))
ax.bar(x, yOptimizedColdstarts, width=width, label="Optimized", bottom=bottom)
bottom += yOptimizedColdstarts
ax.bar(x, yUnoptimizedColdstarts, width=width, label="Unoptimized", bottom=bottom)

ax.set_xlabel("Time (s)")
ax.set_ylabel("Number of cold starts")
plt.grid()

# ax.set_yscale('log')
ax.set_ylim(ymin=0, ymax=14000)
ax.set_xlim(xmin=0, xmax=1200)
fig.set_figwidth(10)
fig.set_figheight(7)

ax.legend(ncol=5, loc='upper center')
plt.savefig("simulation-coldstarts.pdf", bbox_inches='tight')
plt.savefig("simulation-coldstarts.png", bbox_inches='tight')
plt.show()
