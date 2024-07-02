#!/usr/bin/python

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

gv      = np.loadtxt("gv_dc_active_users.txt")
gv_fork = np.loadtxt("gv_fork_active_users.txt")
gv_snap = np.loadtxt("gv_snap_active_users.txt")
cr      = np.loadtxt("cr_active_users.txt")
ph      = np.loadtxt("ph_active_users.txt")

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (10, 4)
plt.plot(gv,      linestyle = ":",                                               linewidth = 3, label = "Graalvisor")
plt.plot(gv_fork, linestyle = "-",  marker = "|", markersize = 10, markevery=10, linewidth = 3, label = "Forking")
plt.plot(ph,      linestyle = "--",                                              linewidth = 3, label = "Photons")
plt.plot(gv_snap, linestyle = "-",                                               linewidth = 3, label = "VM Snapshot")
plt.plot(cr,      linestyle = "-",  marker = "x", markersize = 10, markevery=10, linewidth = 3, label = "OpenWhisk")
plt.xlabel("Time (s)")
plt.ylabel("Active Users")
plt.grid()
plt.legend(ncol=2, loc='lower right')
plt.tight_layout()
plt.savefig("azure-replay-active-users.pdf")
plt.savefig("azure-replay-active-users.png")
plt.show()
