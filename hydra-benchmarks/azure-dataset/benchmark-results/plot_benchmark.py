#!/usr/bin/python

import matplotlib.pyplot as plt
import numpy as np

# Make dotted plot for footprint
gv = np.loadtxt("gv_footprint.txt")
cr = np.loadtxt("cr_footprint.txt")
ph = np.loadtxt("ph_footprint.txt")

fig, axis = plt.subplots()
fig.set_figwidth(5)
fig.set_figheight(3)
plt.plot(gv, linestyle = "solid", label = "Graalvisor")
plt.plot(cr, linestyle = "dashed", label = "OpenWhisk")
plt.plot(ph, linestyle = "dotted", label = "Photons")
plt.legend()
plt.xlabel("Time (s)")
plt.ylabel("Memory (MB)")
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.legend(ncol=3, loc='upper center')
axis.set_ylim(ymax=20000)
fig.tight_layout()
plt.savefig("azure-footprint.pdf")
plt.show()


# Make dotted plot for open requests
gv = np.loadtxt("gv_open_requests.txt")
cr = np.loadtxt("cr_open_requests.txt")
ph = np.loadtxt("ph_open_requests.txt")

plt.plot(gv, linestyle = "dotted", label = "GV")
plt.plot(cr, linestyle = "dotted", label = "CR")
plt.plot(ph, linestyle = "dotted", label = "PH")
plt.legend()
plt.xlabel("Time (s)")
plt.ylabel("Open requests")
plt.show()


# Make dotted plot for active lambdas
gv = np.loadtxt("gv_active_lambdas.txt")
cr = np.loadtxt("cr_active_lambdas.txt")
ph = np.loadtxt("ph_active_lambdas.txt")

plt.plot(gv, linestyle = "dotted", label = "GV")
plt.plot(cr, linestyle = "dotted", label = "CR")
plt.plot(ph, linestyle = "dotted", label = "PH")
plt.legend()
plt.xlabel("Time (s)")
plt.ylabel("Active lambdas")
plt.show()


# Make CDF plot for avg
gv = np.loadtxt("gv_avg_latency.txt")
cr = np.loadtxt("cr_avg_latency.txt")
ph = np.loadtxt("ph_avg_latency.txt")
print("Avg max for GV: " + str(gv.max()))
print("Avg max for CR: " + str(cr.max()))
print("Avg max for PH: " + str(ph.max()))

x_cr = np.sort(cr)
x_gv = np.sort(gv)
x_ph = np.sort(ph)

y_cr = np.arange(len(cr)) / float(len(cr))
y_gv = np.arange(len(gv)) / float(len(gv))
y_ph = np.arange(len(ph)) / float(len(ph))

fig, axis = plt.subplots()
fig.set_figwidth(5)
fig.set_figheight(3)
plt.plot(x_gv, y_gv, linestyle="solid", label='Graalvisor')
plt.plot(x_cr, y_cr, linestyle="dashed",  label='OpenWhisk')
plt.plot(x_ph, y_ph, linestyle="dotted", label='Photons')
plt.ylim(ymin=0, ymax=1)
plt.xlim(xmin=0, xmax=10000)
plt.ylabel('Cumulative Distribution Function')
plt.xlabel('Latency (ms)')
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.legend(ncol=1, loc='lower right')
fig.tight_layout()
plt.savefig("azure-latency.pdf")
plt.show()



# Make CDF plot for max
gv = np.loadtxt("gv_max_latency.txt")
cr = np.loadtxt("cr_max_latency.txt")
ph = np.loadtxt("ph_max_latency.txt")
print("Max max for GV: " + str(gv.max()))
print("Max max for CR: " + str(cr.max()))
print("Max max for PH: " + str(ph.max()))

x_cr = np.sort(cr)
x_gv = np.sort(gv)
x_ph = np.sort(ph)

y_cr = np.arange(len(cr)) / float(len(cr))
y_gv = np.arange(len(gv)) / float(len(gv))
y_ph = np.arange(len(ph)) / float(len(ph))

plt.plot(x_cr, y_cr, label='CR')
plt.plot(x_gv, y_gv, label='GV')
plt.plot(x_ph, y_ph, label='PH')
plt.ylim(ymin=0, ymax=1)
plt.xlim(xmin=0)
plt.ylabel('Cumulative Distribution Function')
plt.xlabel('Max Request Latency (ms)')
plt.legend()
plt.show()
