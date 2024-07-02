#!/usr/bin/python

import results
import matplotlib
import matplotlib.pyplot as plt
import numpy as np


RESULTS_FILE_UNOPTIMIZED = "../results/simulation_d02_unoptimized_keepalive_10min.txt"
RESULTS_FILE_OPTIMIZED = "../results/simulation_d02_10inv_per_1min_keepalive_10min.txt"

unoptimized = np.array(results.read_column(RESULTS_FILE_UNOPTIMIZED, 20))
optimized = np.array(results.read_column(RESULTS_FILE_OPTIMIZED, 20))

diff = unoptimized - optimized

percentages = diff * 100 / unoptimized


print("Memory footprint on average got improved by:")
print(percentages.mean())
