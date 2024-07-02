#!/usr/bin/python3

import numpy
import matplotlib.pyplot as plt

plt.rcParams["figure.figsize"] = (4,5)
plt.rcParams.update({'font.size': 12})

creation_mean = numpy.loadtxt('results-cgroup/experiment-creation-mean.dat')
creation_std  = numpy.loadtxt('results-cgroup/experiment-creation-std.dat')
setup_mean    = numpy.loadtxt('results-cgroup/experiment-setup-mean.dat')
setup_std     = numpy.loadtxt('results-cgroup/experiment-setup-std.dat')
adding_mean   = numpy.loadtxt('results-cgroup/experiment-adding-mean.dat')
adding_std    = numpy.loadtxt('results-cgroup/experiment-adding-std.dat')
removing_mean = numpy.loadtxt('results-cgroup/experiment-removing-mean.dat')
removing_std  = numpy.loadtxt('results-cgroup/experiment-removing-std.dat')
labels        = numpy.loadtxt('results-cgroup/experiment-procs.dat', dtype='str')

plt.bar(labels, creation_mean, label='Cgroup creation')
bottom = creation_mean
plt.bar(labels, setup_mean, label='Cgroup setup', bottom=bottom)
bottom += setup_mean
plt.bar(labels, adding_mean, label='Cgroup adding', bottom=bottom)
bottom += adding_mean
plt.bar(labels, removing_mean, label='Cgroup removing', bottom=bottom)

plt.ylim(ymin=0)
plt.legend()
plt.xlabel('Concurrent invocations')
plt.ylabel('Time (us)')
plt.tight_layout()
plt.savefig('run-cgroup.pdf')
plt.savefig('run-cgroup.png', dpi=300)
