#!/usr/bin/python

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

def numpy_ewma_vectorized(data, window):

    alpha = 2 /(window + 1.0)
    alpha_rev = 1-alpha

    scale = 1/alpha_rev
    n = data.shape[0]

    r = np.arange(n)
    scale_arr = scale**r
    offset = data[0]*alpha_rev**(r+1)
    pw0 = alpha*alpha_rev**(n-1)

    mult = data*pw0*scale_arr
    cumsums = mult.cumsum()
    out = offset + cumsums*scale_arr[::-1]
    return out

gv      = numpy_ewma_vectorized(np.loadtxt("gv_dc_throughput.txt"),   10)
gv_fork = numpy_ewma_vectorized(np.loadtxt("gv_fork_throughput.txt"), 10)
gv_snap = numpy_ewma_vectorized(np.loadtxt("gv_snap_throughput.txt"), 10)
cr      = numpy_ewma_vectorized(np.loadtxt("cr_throughput.txt"),      10)
ph      = numpy_ewma_vectorized(np.loadtxt("ph_throughput.txt"),      10)

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (10, 4)
plt.plot(gv,      linestyle = ":",                                               linewidth = 3, label = "Graalvisor")
plt.plot(gv_fork, linestyle = "-",  marker = "|", markersize = 10, markevery=10, linewidth = 3, label = "Forking")
plt.plot(ph,      linestyle = "--",                                              linewidth = 3, label = "Photons")
plt.plot(gv_snap, linestyle = "-",                                               linewidth = 3, label = "VM Snapshot")
plt.plot(cr,      linestyle = "-",  marker = "x", markersize = 10, markevery=10, linewidth = 3, label = "OpenWhisk")
plt.xlabel("Time (s)")
plt.ylabel("Throughput (ops/s)")
plt.grid()
plt.ylim(ymin=0, ymax=20)
plt.xlim(xmin=0, xmax=1200)
#plt.xlim(xmin=100, xmax=200)
#plt.legend(ncol=4, loc='upper right')
plt.tight_layout()
plt.savefig("azure-replay-throughput.pdf")
plt.savefig("azure-replay-throughput.png")
plt.show()
