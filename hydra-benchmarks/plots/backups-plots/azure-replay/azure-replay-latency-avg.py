#!/usr/bin/python

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

gv      = np.loadtxt("gv_dc_avg_latency.txt")
gv_fork = np.loadtxt("gv_fork_avg_latency.txt")
gv_snap = np.loadtxt("gv_snap_avg_latency.txt")
cr      = np.loadtxt("cr_avg_latency.txt")
ph      = np.loadtxt("ph_avg_latency.txt")

x_gv      = np.sort(gv)
x_cr      = np.sort(cr)
x_gv_fork = np.sort(gv_fork)
x_gv_snap = np.sort(gv_snap)
x_ph      = np.sort(ph)

y_gv      = np.arange(len(gv)) / float(len(gv))
y_cr      = np.arange(len(cr)) / float(len(cr))
y_gv_fork = np.arange(len(gv_fork)) / float(len(gv_fork))
y_gv_snap = np.arange(len(gv_snap)) / float(len(gv_snap))
y_ph      = np.arange(len(ph)) / float(len(ph))

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (10, 4)
plt.plot(x_gv,      y_gv,      linestyle = ":",                                               linewidth = 3, label = "Graalvisor")
plt.plot(x_gv_fork, y_gv_fork, linestyle = "-",  marker = "|", markersize = 10, markevery=10, linewidth = 3, label = "Forking")
plt.plot(x_ph,      y_ph,      linestyle = "--",                                              linewidth = 3, label = "Photons")
plt.plot(x_gv_snap, y_gv_snap, linestyle = "-",                                               linewidth = 3, label = "VM Snapshot")
plt.plot(x_cr,      y_cr,      linestyle = "-",  marker = "x", markersize = 10, markevery=10, linewidth = 3, label = "OpenWhisk")
plt.ylim(ymin=0, ymax=1)
plt.xlim(xmin=0, xmax=80000)
plt.xlabel("User Request Latency (ms)")
plt.ylabel("CDF")
plt.grid()
plt.legend(ncol=2, loc='lower right')
plt.tight_layout()
plt.savefig("azure-replay-latency-avg.pdf")
plt.savefig("azure-replay-latency-avg.png")
plt.show()
