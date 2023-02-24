#!/usr/bin/python3

import argparse
import numpy
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()
parser.add_argument('--option', dest="option", type=str)
parser.add_argument('--to-test', dest="to_test", type=str)
args = parser.parse_args()

mean = numpy.loadtxt(f'results/{args.option}-{args.to_test}-mean.dat')
stdev  = numpy.loadtxt(f'results/{args.option}-{args.to_test}-stdev.dat')
labels      = numpy.loadtxt('results/num.dat', dtype='str')

if args.option == "domain":
    plt.bar(labels, mean, label='pkey_mprotect')
else:
    plt.bar(labels, mean, label='pkey_set')

plt.ylim(ymin=0)
plt.legend()
plt.xlabel(f'Number of {args.to_test}')
plt.ylabel('Time (ns)')
plt.savefig(f'{args.option}-{args.to_test}.pdf')
