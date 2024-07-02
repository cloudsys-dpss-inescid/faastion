#!/usr/bin/python3

import numpy
import matplotlib.pyplot as plt

chroot_mean = numpy.loadtxt('results-chroot/experiment-mean.dat')
chroot_std  = numpy.loadtxt('results-chroot/experiment-std.dat')
labels      = numpy.loadtxt('results-chroot/experiment-procs.dat', dtype='str')

plt.rcParams["figure.figsize"] = (4,5)
plt.rcParams.update({'font.size': 12})
plt.bar(labels, chroot_mean, label='Chroot')
plt.ylim(ymin=0)
plt.legend()
plt.xlabel('Concurrent invocations')
plt.ylabel('Time (us)')
plt.tight_layout()
plt.savefig('run-chroot.pdf')
plt.savefig('run-chroot.png', dpi=300)
