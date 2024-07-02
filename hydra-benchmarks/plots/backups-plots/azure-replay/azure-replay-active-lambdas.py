#!/usr/bin/python

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

gv      = np.loadtxt("gv_dc_active_lambdas.txt")
gv_fork = np.loadtxt("gv_fork_active_lambdas.txt")
cr      = np.loadtxt("cr_active_lambdas.txt")
ph      = np.loadtxt("ph_active_lambdas.txt")
gv_snap = np.loadtxt("gv_snap_active_lambdas.txt")

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (10, 4)
plt.plot(gv,      linestyle = ":",                                               linewidth = 3, label = "Graalvisor")
plt.plot(gv_fork, linestyle = "-",  marker = "|", markersize = 10, markevery=10, linewidth = 3, label = "Forking")
plt.plot(ph,      linestyle = "--",                                              linewidth = 3, label = "Photons")
plt.plot(gv_snap, linestyle = "-",                                               linewidth = 3, label = "VM Snapshot")
plt.plot(cr,      linestyle = "-",  marker = "x", markersize = 10, markevery=10, linewidth = 3, label = "OpenWhisk")
plt.xlabel("Time (s)")
plt.ylabel("Active microVMs")
plt.grid()
plt.xlim(xmin=0, xmax=1200)
#plt.legend(ncol=2, loc='lower right')
plt.tight_layout()
plt.savefig("azure-replay-active-lambdas.pdf")
plt.savefig("azure-replay-active-lambdas.png")
plt.show()
