#!/usr/bin/python

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

gv      = np.loadtxt("gv_dc_open_requests.txt")
gv_fork = np.loadtxt("gv_fork_open_requests.txt")
gv_snap = np.loadtxt("gv_snap_open_requests.txt")
cr      = np.loadtxt("cr_open_requests.txt")
ph      = np.loadtxt("ph_open_requests.txt")

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (10, 4)
plt.plot(gv,      linestyle = ":",                                               linewidth = 3, label = "Graalvisor")
plt.plot(gv_fork, linestyle = "-",  marker = "|", markersize = 10, markevery=10, linewidth = 3, label = "Forking")
plt.plot(ph,      linestyle = "--",                                              linewidth = 3, label = "Photons")
plt.plot(gv_snap, linestyle = "-",                                               linewidth = 3, label = "VM Snapshot")
plt.plot(cr,      linestyle = "-",  marker = "x", markersize = 10, markevery=10, linewidth = 3, label = "OpenWhisk")
plt.xlabel("Time (s)")
plt.ylabel("Open Requests")
plt.grid()
plt.legend(ncol=2, loc='upper right')
plt.tight_layout()
plt.savefig("azure-replay-open-requests.pdf")
plt.savefig("azure-replay-open-requests.png")
plt.show()
