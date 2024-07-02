#!/usr/bin/python3

import matplotlib
from matplotlib import pyplot as plt

# Isolate, CPython, V8, JVM, Firacracker, Docker, Qemu
x =    [1, 1.5, 8.72, 36.56, 38.32, 63.54, 46.95, 118.54] # Memory
y =    [0.2, 0.7, 13.1, 40.8, 26.9, 166.7, 257.1, 241.7] # Latency

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

plt.annotate('Graalvisor Isolate', xy=(x[0] + .125, y[0]))
plt.annotate('Graalvisor Sandbox', xy=(x[1] + .125, y[1]))
plt.annotate('CPython', xy=(x[2] + 2.5 , y[2] - 5))
plt.annotate('NodeJS', xy=(x[3] - 27.5, y[3] - 2.5))
plt.annotate('JVM', xy=(x[4] + 15, y[4] - 2.5))
plt.annotate('Firecracker', xy=(x[5] + 20, y[5] - 50))
plt.annotate("Docker", xy=(x[6] - 35, y[6] + 20))
#plt.annotate("Qemu", xy=(x[6] - 25, y[6]), xytext=(x[6] - 110, y[6]), arrowprops={"arrowstyle":"->", "color":"gray"})
plt.annotate("QEMU", xy=(x[7] + 15, y[7] + 20))

#plt.fill_betweenx([0, 200], [0, 0], [100, 200])
#plt.fill_between([100, 200], [1, 1], [1000, 1000])

plt.savefig('virtualization-performance.pdf')
#plt.show()
