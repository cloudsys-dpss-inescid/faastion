#!/usr/bin/python

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

rutimes = ['Isolate', 'GV', 'PY', 'JS', 'JV', 'GV', 'PY', 'JS', 'JV']
x = np.arange(len(rutimes))

avg_baremetal_startup = [0.47, 12.2, 13.1, 40.8, 26.9, 262, 1918, 2009, 2050 ] 

width = .25
fig, ax1 = plt.subplots()
#ax1.bar(x, avg_baremetal_startup, width, color='black', hatch='x')
ax1.bar(x, avg_baremetal_startup, width, alpha=0.75)
ax1.set_xticks(x, rutimes)
ax1.set_ylabel('Cold start latency (ms)')
ax1.set_yscale('log')
ax1.set_ylim(ymin=.1)

# Baremetal runtimes
for i in range(1, 5):
#    ax1.get_children()[i].set_color("red")
    ax1.get_children()[i].set_hatch("//")

# MicroVM-ed runtimes
for i in range(5, 9):
#    ax1.get_children()[i].set_color("blue")
    ax1.get_children()[i].set_hatch("..")

legends = [
    matplotlib.patches.Patch(hatch="//", label="Bare-metal Runtimes", alpha=0.75),
    matplotlib.patches.Patch(hatch="..", label="microVM Runtimes", alpha=0.75),
]

fig.set_figwidth(5)
fig.set_figheight(3)
ax1.legend(handles=legends, prop={"size": 10})
ax1.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.savefig("cold-start-latency.pdf")
plt.show()
