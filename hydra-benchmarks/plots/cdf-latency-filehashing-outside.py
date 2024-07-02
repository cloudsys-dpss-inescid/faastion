#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt

def read_benchmark_latency(path):
    benchmark_latency = []
    with open(path) as file:
        for line in file:
            if 'Time taken:' in line:
                benchmark_latency.append(int(line.split()[2]))
    return benchmark_latency


cr = read_benchmark_latency('../results/java/cr-file-hashing-test-10-1-2048/app.log')
gv = read_benchmark_latency('../results/java/gv-file-hashing-niuk-test-10-1-2048/app.log')

x_cr = np.sort(cr)
x_gv = np.sort(gv)
  
y_cr = np.arange(len(cr)) / float(len(cr))
y_gv = np.arange(len(gv)) / float(len(gv))

fig, axes = plt.subplots(nrows=1, ncols=2)
axes[0].plot(x_cr, y_cr, marker='o', label='JVM (microVM)')
axes[0].plot(x_gv, y_gv, marker='x', label='GV (microVM)')
axes[0].set_ylim(ymin=0, ymax=1)
axes[0].set_xlim(xmin=0)
axes[0].set_ylabel('Cumulative Distribution Function')
axes[0].set_xlabel('Request Latency (us)')
axes[0].set_axisbelow(True)
axes[0].grid(axis = 'y', linestyle = '--', linewidth = 0.25)
axes[0].legend()

# ../results/java/gv-file-hashing-svm/lambda.rss (with and without js+py)
# ../results/java/gv-file-hashing-niuk/lambda.rss
# Taken by hand by running docker run and curling.
# ../results/java/cr-file-hashing/lambda.rss
y = [34724, 71836, 179380, 46872, 223460 ]
axes[1].set_ylabel('Memory Footprint (KBs)')
axes[1].bar(['GV-JV', 'GV', 'GV\n(microVM)', 'JVM', 'JVM\n(microVM)'], y, width = .4)
axes[1].set_axisbelow(True)
axes[1].grid(axis = 'y', linestyle = '--', linewidth = 0.25)

fig.tight_layout()
#plt.show()
plt.savefig("cdf-latency-filehashing.pdf")
