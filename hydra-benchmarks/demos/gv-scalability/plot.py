#!/usr/bin/python

import matplotlib.pyplot as plt
import numpy as np

isolates = [1, 2, 4, 8, 16, 32, 64, 128]

def plot(mode):
  x = np.arange(len(isolates))
  avg_latency = []
  avg_memory = []

  for isolate in isolates:
    latency = np.loadtxt('results/latency-' + mode + '-' + str(isolate) + '.dat', delimiter='\n')
    avg_latency.append(np.average(latency))
    memory = np.loadtxt('results/pss-' + mode + '-' + str(isolate) + '.dat', delimiter='\n')
    memory = memory / isolate
    avg_memory.append(np.average(memory))

  width = .25
  fig, ax1 = plt.subplots()
  ax1.bar(x - width/2, avg_latency, width, label='Sandbox Startup', hatch='o')
  ax1.set_xticks(x, isolates)
  ax1.set_ylabel('Sandbox startup time (ms)')
  ax1.set_xlabel('Number of sandboxes')
  ax2 = ax1.twinx()
  ax2.bar(x + width/2, avg_memory, width, label='Sandbox Footprint', color='red', hatch='x')
  ax2.set_ylabel('Memory Footprint (KB)')
  ax1.set_axisbelow(True)
  ax2.set_axisbelow(True)
  ax1.grid(axis = 'y', linestyle = '--', linewidth = 0.25)

  fig.legend(loc="upper right", bbox_to_anchor=(.82,.95))
  plt.tight_layout()
  plt.savefig('gv-' + mode + '-scalability.pdf')

plot('process')
plot('thread')
