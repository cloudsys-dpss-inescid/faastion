#!/usr/bin/python

import numpy as np
import matplotlib.pyplot as plt

mem_sharing   = np.loadtxt('mem_sharing.dat')
lat_sharing   = np.loadtxt('lat_sharing.dat')
req_sharing   = np.loadtxt('req_sharing.dat')
mem_nosharing = np.loadtxt('mem_nosharing.dat')
lat_nosharing = np.loadtxt('lat_nosharing.dat')
req_nosharing = np.loadtxt('req_nosharing.dat')

x_sharing   = np.arange(0, len(mem_sharing),   1)
x_nosharing = np.arange(0, len(mem_nosharing), 1)

fig, axes = plt.subplots(nrows=3, ncols=1)
axes[0].plot(x_sharing,   mem_sharing,   marker='o', label='Sharing')
axes[0].plot(x_nosharing, mem_nosharing, marker='x', label='No Sharing')
axes[0].set_ylim(ymin=0)
axes[0].set_xlim(xmin=0)
axes[0].set_ylabel('Memory (MBs)')
axes[0].set_xlabel('Concurrent Contexts')
axes[0].set_axisbelow(True)
axes[0].grid(axis = 'y', linestyle = '--', linewidth = 0.25)

axes[1].plot(x_sharing,   lat_sharing,   marker='o', label='Sharing')
axes[1].plot(x_nosharing, lat_nosharing, marker='x', label='No Sharing')
axes[1].set_ylim(ymin=0)
axes[1].set_xlim(xmin=0)
axes[1].set_ylabel('Latency (ms)')
axes[1].set_xlabel('Concurrent Contexts')
axes[1].set_axisbelow(True)
axes[1].grid(axis = 'y', linestyle = '--', linewidth = 0.25)


axes[2].plot(x_sharing,   req_sharing,   marker='o', label='Sharing')
axes[2].plot(x_nosharing, req_nosharing, marker='x', label='No Sharing')
axes[2].set_ylim(ymin=0)
axes[2].set_xlim(xmin=0)
axes[2].set_ylabel('Latency (ms)')
axes[2].set_xlabel('Request Number')
axes[2].legend()
axes[2].set_axisbelow(True)
axes[2].grid(axis = 'y', linestyle = '--', linewidth = 0.25)


fig.tight_layout()
#plt.show()
plt.savefig("engine-sharing.pdf")
