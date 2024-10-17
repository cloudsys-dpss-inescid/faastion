#!/usr/bin/python3

import numpy as np
import matplotlib.pyplot as plt

# Load data from files
transitions = np.loadtxt('results/transitions.dat')
percentages = np.loadtxt('results/percentages.dat')
untrusted = np.loadtxt('results/untrusted.dat')
benchmarks = np.loadtxt('results/benchmarks.dat', dtype=str)  # Load benchmarks as strings

x = np.arange(len(benchmarks))  # X-axis positions for benchmarks

# Set plot style and figure size
plt.rcParams.update({'font.size': 10})
plt.rcParams["figure.figsize"] = (8, 4)

# Bar graph 1: Transitions per second
fig, ax = plt.subplots()
ax.bar(x, transitions, width=0.6, color='blue', label="Transitions per second")
ax.set_xticks(x)
ax.set_xticklabels(benchmarks)
ax.set_ylim(ymin=0)
ax.set_ylabel("Number of transitions per second")
ax.set_xlabel("Benchmarks")
ax.grid(axis='y', linestyle='--', linewidth=0.5)
plt.title("Transitions per Second")
plt.tight_layout()
plt.savefig("results/transitions-per-second.pdf", dpi=300)

# Bar graph 2: Percentage of time in native code
fig, ax = plt.subplots()
ax.bar(x, percentages, width=0.6, color='red', label="% of time in native code")
ax.set_xticks(x)
ax.set_xticklabels(benchmarks)
ax.set_ylim(ymin=0)
ax.set_ylabel("Percentage of time in native code")
ax.set_xlabel("Benchmarks")
ax.grid(axis='y', linestyle='--', linewidth=0.5)
plt.title("Percentage of Time in Native Code")
plt.tight_layout()
plt.savefig("results/percentage-native-code.pdf", dpi=300)

# Bar graph 3: Untrusted methods
fig, ax = plt.subplots()
ax.bar(x, untrusted, width=0.6, color='green', label="% of untrusted native methods")
ax.set_xticks(x)
ax.set_xticklabels(benchmarks)
ax.set_ylim(ymin=0)
ax.set_ylabel("Percentage of untrusted native methods")
ax.set_xlabel("Benchmarks")
ax.grid(axis='y', linestyle='--', linewidth=0.5)
plt.title("Percentage of Untrusted Native Methods")
plt.tight_layout()
plt.savefig("results/untrusted-methods.pdf", dpi=300)
