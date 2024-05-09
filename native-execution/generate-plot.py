#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

transitions = np.loadtxt('results/transitions.dat')
percentages = np.loadtxt('results/percentages.dat')
labels      = np.genfromtxt('results/benchmarks.dat', dtype=str)
x           = np.arange(len(labels))

#transitions[3]=0
#percentages[3]=0
#labels[3]=""

#transitions[4]=0
#percentages[4]=0
#labels[4]=""

plt.rcParams.update({'font.size': 10})
plt.rcParams["figure.figsize"] = (8,4)

width = .25
fig, ax1 = plt.subplots()

ax1.bar(x - (width * 1.05)/2, transitions, width, label="Transitions per second")
ax1.set_xticks(x, labels)
ax1.set_ylim(ymin=0, ymax=30)
ax1.set_ylabel("Number of transitions per second")
ax1.grid(axis = 'y', linestyle = '--', linewidth = 0.25)

ax2 = ax1.twinx()
ax2.bar(x + (width * 1.05)/2, percentages, width, color="red", label="% of time in native code")
ax2.set_ylabel("Percentage of time in native code")
ax2.set_ylim(ymin=0, ymax=30)

fig.legend(ncol=2, bbox_to_anchor=(.65,.96))
plt.tight_layout()
plt.savefig("native-execution.pdf")
plt.savefig("native-execution.png", dpi=300)
