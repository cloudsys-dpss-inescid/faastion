#!/usr/bin/python

import sys
import csv
import io
import matplotlib.pyplot as plt
import argparse

# Syntax example: cat data.txt | plotter.py
# data.txt should be a file with six columns.


def read_stdin_xy():
  # Read input from stdin.
  input = sys.stdin.read()

  # Read string as a csv file (line by line, separated by a delimiter).
  reader = csv.reader(io.StringIO(input), delimiter=' ')

  x = []
  yInvocations = []
  yColdstarts = []
  yRunningUsers = []
  yRunningFunctions = []
  yRunningInvocations = []
  yRunningFootprint = []
  yCachedUsers = []
  yCachedFunctions = []
  yCachedFootprint = []
  yOptimizedColdstarts = []
  yRunningOptimizedFunctions = []

  # For each line, split and add to x and or y.
  for cols in reader:
    try:
      x.append(float(cols[1]))
      yInvocations.append(float(cols[4]))
      yColdstarts.append(float(cols[6]))
      yRunningUsers.append(float(cols[10]))
      yRunningFunctions.append(float(cols[12]))
      yRunningInvocations.append(float(cols[14]))
      yRunningFootprint.append(float(cols[16]))
      yCachedUsers.append(float(cols[20]))
      yCachedFunctions.append(float(cols[22]))
      yCachedFootprint.append(float(cols[24]))
      yOptimizedColdstarts.append(float(cols[27]))
      yRunningOptimizedFunctions.append(float(cols[29]))
    except Exception as e:
      print("Warning ignoring " + e)
  return x, yInvocations, yColdstarts, yRunningUsers, yRunningFunctions, yRunningInvocations, yRunningFootprint, yCachedUsers, yCachedFunctions, yCachedFootprint, yOptimizedColdstarts, yRunningOptimizedFunctions


parser = argparse.ArgumentParser(description='Plot two-dimensional datapoints.')
parser.add_argument('-o', '--output', required=False)
parser.add_argument('-x', '--xlabel', required=False)
parser.add_argument('-y', '--ylabel', required=False)
parser.add_argument('-ymax', '--ymax', required=False)

args = parser.parse_args()

x, yInvocations, yColdstarts, yRunningUsers, yRunningFunctions, yRunningInvocations, yRunningFootprint, yCachedUsers, yCachedFunctions, yCachedFootprint, yOptimizedColdstarts, yRunningOptimizedFunctions = read_stdin_xy()

# Adjust startin point.
first = x[0]
x = [elem - first for elem in x]

fig = plt.figure()
ax1 = fig.add_subplot(111)

ax1.scatter(x, yInvocations, s=1, label='Invocations')
ax1.scatter(x, yColdstarts, s=1, label='Cold starts')
ax1.scatter(x, yOptimizedColdstarts, s=1, label='Optimized cold starts')
ax1.scatter(x, yRunningUsers, s=1, label='Running users')
ax1.scatter(x, yRunningFunctions, s=1, label='Running functions')
ax1.scatter(x, yRunningOptimizedFunctions, s=1, label='Running optimized functions')
ax1.scatter(x, yRunningInvocations, s=1, label='Running invocations')
ax1.scatter(x, yRunningFootprint, s=1, label='Running Footprint') # TODO - show on another axis
ax1.scatter(x, yCachedUsers, s=1, label='Cached users')
ax1.scatter(x, yCachedFunctions, s=1, label='Cached functions')
ax1.scatter(x, yCachedFootprint, s=1, label='Cached footprint') # TODO - show on another axis

plt.legend(loc='upper left')

# Plot.
plt.ylim(ymin=0)#, ymax=float(args.ymax))
plt.xlim(xmin=0)

if args.xlabel is not None:
  plt.xlabel(args.xlabel)

if args.ylabel is not None:
  plt.ylabel(args.ylabel)

if args.output is not None:
  plt.savefig(args.output)
else:
  plt.show()

