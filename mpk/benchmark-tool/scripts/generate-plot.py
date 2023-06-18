#!/usr/bin/python3

import argparse
import numpy
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()
parser.add_argument('--mean',    dest="mean",    type=str)
parser.add_argument('--std',     dest="std",     type=str)
parser.add_argument('--xvalues', dest="xvalues", type=str)
parser.add_argument('--ylabel',  dest="ylabel",  type=str)
parser.add_argument('--xlabel',  dest="xlabel",  type=str)
parser.add_argument('--plot',    dest="plot",  type=str)
args = parser.parse_args()

mean    = numpy.loadtxt(args.mean)
stdev   = numpy.loadtxt(args.std)
xvalues = numpy.loadtxt(args.xvalues, dtype='str')
plt.bar(xvalues, mean)

plt.ylim(ymin=0)
plt.xlabel(args.xlabel)
plt.ylabel(args.ylabel)
plt.savefig(f'{args.plot}.pdf')
plt.savefig(f'{args.plot}.png', dpi=300)
