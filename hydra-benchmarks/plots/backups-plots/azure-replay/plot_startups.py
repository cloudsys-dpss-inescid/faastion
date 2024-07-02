#!/usr/bin/python

import matplotlib.pyplot as plt
import numpy as np

# Make dotted plot for footprint
cr = np.loadtxt("cr_startups.txt")
gv_dc = np.loadtxt("gv_dc_startups.txt")
ph = np.loadtxt("ph_startups.txt")

fig, axis = plt.subplots()
fig.set_figwidth(5)
fig.set_figheight(3)
plt.plot(cr, linestyle = "dashed", label = "OpenWhisk")
plt.plot(gv_dc, linestyle = "solid", label = "Graalvisor DC")
plt.plot(ph, linestyle = "dotted", label = "Photons")
plt.legend()
plt.xlabel("Time (s)")
plt.ylabel("Startup")
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.legend(ncol=3, loc='upper center')
# axis.set_ylim(ymax=20000)
fig.tight_layout()
plt.show()
