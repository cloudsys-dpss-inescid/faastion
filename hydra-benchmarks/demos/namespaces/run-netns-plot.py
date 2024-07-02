#!/usr/bin/python3

import numpy
import matplotlib.pyplot as plt

plt.rcParams["figure.figsize"] = (4,5)
plt.rcParams.update({'font.size': 12})

labels = numpy.loadtxt('results-netns/experiment-procs.dat', dtype='str')

netnscreation_mean = numpy.loadtxt('results-netns/experiment-netnscreation-mean.dat')
plt.bar(labels, netnscreation_mean, label='Netns creation')
bottom = netnscreation_mean

tapcreation_mean = numpy.loadtxt('results-netns/experiment-tapcreation-mean.dat')
plt.bar(labels, tapcreation_mean, label='Tap creation', bottom=bottom)
bottom += tapcreation_mean

vethcreation_mean = numpy.loadtxt('results-netns/experiment-vethcreation-mean.dat')
plt.bar(labels, vethcreation_mean, label='Veth creation', bottom=bottom)
bottom += vethcreation_mean

vethsetup_mean = numpy.loadtxt('results-netns/experiment-vethsetup-mean.dat')
plt.bar(labels, vethsetup_mean, label='Veth setup', bottom=bottom)
bottom += vethsetup_mean

routessetup_mean = numpy.loadtxt('results-netns/experiment-routessetup-mean.dat')
plt.bar(labels, routessetup_mean, label='Routes setup', bottom=bottom)
bottom += routessetup_mean

deletion_mean = numpy.loadtxt('results-netns/experiment-deletion-mean.dat')
plt.bar(labels, deletion_mean, label='Netns deletion', bottom=bottom)

plt.ylim(ymin=0)
plt.legend()
plt.xlabel('Concurrent invocations')
plt.ylabel('Time (us)')
plt.tight_layout()
plt.savefig('run-netns.pdf')
plt.savefig('run-netns.png', dpi=300)
