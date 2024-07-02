#!/usr/bin/python

import sys
import csv
import io
import matplotlib
import matplotlib.pyplot as plt

# Syntax example: cat plot-data.txt | plot-simulation.py
# data.txt should be a file with six columns.


def read_stdin_xy():
  # Read input from stdin.
  input = sys.stdin.read()

  # Read string as a csv file (line by line, separated by a delimiter).
  reader = csv.reader(io.StringIO(input), delimiter=' ')

  x = []
  yActiveUsers = []
  yActiveFunctions = []
  yCachedActiveUsers = []
  yCachedActiveFunctions = []
  yRunningInvocations = []

  # For each line, split and add to x and or y.
  for cols in reader:
    try:
      xentry = float(cols[0])
      activeUsersEntry = float(cols[1])
      activeFunctionsEntry = float(cols[2])
      cachedActiveUsersEntry = float(cols[3])
      cachedActiveFunctionsEntry = float(cols[4])
      runningInvocationsEntry = float(cols[5])
      x.append(xentry)
      yActiveUsers.append(activeUsersEntry)
      yActiveFunctions.append(activeFunctionsEntry)
      yCachedActiveUsers.append(cachedActiveUsersEntry)
      yCachedActiveFunctions.append(cachedActiveFunctionsEntry)
      yRunningInvocations.append(runningInvocationsEntry)
    except Exception as e:
      print("Warning ignoring " + str(cols))
  return x, yActiveUsers, yActiveFunctions, yCachedActiveUsers, yCachedActiveFunctions, yRunningInvocations


x, activeUsers, activeFunctions, cachedActiveUsers, cachedActiveFunctions, runningInvocations = read_stdin_xy()
first = x[0]
x = [elem - first for elem in x]

# Convert from ms to sec
x = [elem/1000 for elem in x]

matplotlib.rcParams.update({'font.size': 16})
plt.rcParams["figure.figsize"] = (10, 4)
plt.xlabel('Time (seconds)')
plt.ylabel('# Functions, Users, Invocations')
plt.plot(x[::500], activeFunctions[::500],       linewidth=3, marker="|", markersize=25, markevery=10, label='Active Functions')
plt.plot(x[::500], activeUsers[::500],           linewidth=3, linestyle=":",                           label='Active Users')
plt.plot(x[::500], runningInvocations[::500],    linewidth=3, label='Invocations')
plt.plot(x[::500], cachedActiveUsers[::500],     linewidth=3, marker="x", markersize=25, markevery=10, label='Cached Users')
plt.plot(x[::500], cachedActiveFunctions[::500], linewidth=3, linestyle="--",                          label='Cached Functions')
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.yscale('log', base=10)
plt.ylim(ymin=0)
plt.xlim(xmin=0, xmax=600)
plt.legend(loc='lower left', ncol=2)
plt.savefig("azure-users-functions.pdf", bbox_inches='tight')
plt.savefig("azure-users-functions.png", bbox_inches='tight', dpi=300)
plt.show()
