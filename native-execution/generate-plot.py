#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

transitions = np.loadtxt('results/transitions.dat')
percentages = np.loadtxt('results/percentages.dat')
labels      = ["ML inference", "Hashing", "Hello World", "REST", "Video\n encoding", "Micronaut"]
x           = np.arange(len(labels))

width = .25
fig, ax1 = plt.subplots()

ax1.bar(x - (width * 1.05)/2, transitions, width, label="Transitions per sec")
ax1.set_xticks(x, labels)
ax1.set_ylim(ymin=0)
ax1.set_ylabel("Number of transitions per second")
ax1.grid(axis = 'y', linestyle = '--', linewidth = 0.25)

ax2 = ax1.twinx()
ax2.bar(x + (width * 1.05)/2, percentages, width, color="red", label="% Native Code")
ax2.set_ylabel("Percentage of time in native code")
ax2.set_ylim(ymin=0, ymax=100)

fig.legend(bbox_to_anchor=(.85,.95))
plt.tight_layout()
plt.savefig("native-execution.pdf")
plt.savefig("native-execution.png")
