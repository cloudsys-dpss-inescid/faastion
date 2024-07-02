#!/usr/bin/python3

import matplotlib
from matplotlib import pyplot as plt

# Isolate, Graalvisor, CPython, V8, JVM, Firacracker, Docker, Qemu
x =    [0.48, 14.74, 8.72, 36.56, 38.32, 63.54, 46.95, 118.54] # Memory
y =    [0.25, 05.00, 13.1, 40.80, 26.90, 166.7, 257.1, 241.70] # Latency

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (8, 5.5)

plt.xlabel('Memory Footprint (MB)')
plt.ylabel('Sartup Latency (ms)')

plt.xlim(0.11, 1000)
plt.ylim(0.11, 1000)

plt.yscale('log', base=10)
plt.xscale('log', base=10)

plt.grid()

plt.plot(x, y, "o", markersize=10, markerfacecolor="blue")

plt.annotate('Graalvisor Sandbox', xy=(x[0] + .125, y[0] - 0.1))
plt.annotate('Graalvisor', xy=(x[1] + 4.5, y[1]))
plt.annotate('CPython', xy=(x[2] - 7, y[2] - 2))
plt.annotate('NodeJS', xy=(x[3] - 27.5, y[3] - 2.5))
plt.annotate('JVM', xy=(x[4] + 15, y[4] - 2.5))
plt.annotate('Firecracker', xy=(x[5] + 15, y[5] - 75))
plt.annotate("Docker", xy=(x[6] - 35, y[6] + 20))
plt.annotate("QEMU", xy=(x[7] + 15, y[7] + 30))

#plt.axvline(x=125)
#plt.axhline(y=750)
#plt.fill_betweenx([0, 200], [0, 0], [100, 200])
#plt.fill_between([100, 200], [1, 1], [1000, 1000])

plt.savefig('virtualization-performance.pdf')
plt.savefig('virtualization-performance.png')
#plt.show()
