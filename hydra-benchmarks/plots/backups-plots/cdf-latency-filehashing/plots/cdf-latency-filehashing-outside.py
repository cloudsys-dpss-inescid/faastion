#!/usr/bin/python

import numpy as np
import matplotlib
import matplotlib.pyplot as plt

def read_benchmark_latency(path):
    benchmark_latency = []
    with open(path) as file:
        for line in file:
            if 'Time taken:' in line:
                benchmark_latency.append(int(line.split()[2]))
    return benchmark_latency

cr = read_benchmark_latency('../results/java/cr-file-hashing-test-25-1-2048/1/app.log')
gv = read_benchmark_latency('../results/java/gv-file-hashing-container-isolate-test-25-1-2048/1/app.log')

# From us to ms.
cr = np.array(cr) / 1000
gv = np.array(gv) / 1000

x_cr = np.sort(cr)
x_gv = np.sort(gv)
  
y_cr = np.arange(len(cr)) / float(len(cr))
y_gv = np.arange(len(gv)) / float(len(gv))

# Since we don't at least 100 datapoints, we need to fix the last one to 1.
y_cr[-1] = 1
y_gv[-1] = 1

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (10, 4)

fig, axes = plt.subplots(nrows=1, ncols=2)
axes[0].plot(x_cr, y_cr, marker='o', label='OpenWhisk JVM')
axes[0].plot(x_gv, y_gv, marker='x', label='Graalvisor')
axes[0].set_ylim(ymin=0, ymax=1)
axes[0].set_xlim(xmin=0)
axes[0].set_ylabel('Cumulative Distribution Function')
axes[0].set_xlabel('Request Latency (ms)')
axes[0].set_axisbelow(True)
axes[0].grid(axis = 'y', linestyle = '--', linewidth = 0.25)
axes[0].legend()

# RSS results taken from docker stats while sleep infinity between each round.
# Measure memory does not work yet with container backend so we had to measure manually.
rss_gv = np.array([14.5, 10.6, 12.57, 12.57, 14.58])
rss_cr = np.array([23.8, 26.3, 23.76, 24.23, 23.79])
rss_avg  = [rss_gv.mean(), rss_cr.mean()]
rss_std  = [rss_gv.std(), rss_cr.std()]
axes[1].set_ylabel('Memory Footprint (MBs)')
axes[1].bar(['Graalvisor', 'OpenWhisk JVM'], rss_avg, yerr=rss_std, width = .4)
axes[1].set_axisbelow(True)
axes[1].grid(axis = 'y', linestyle = '--', linewidth = 0.25)

plt.savefig("cdf-latency-filehashing.pdf", bbox_inches='tight')
plt.savefig("cdf-latency-filehashing.png", bbox_inches='tight')
plt.show()
