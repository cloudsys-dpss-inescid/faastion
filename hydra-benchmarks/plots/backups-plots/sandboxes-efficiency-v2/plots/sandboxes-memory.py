#!/usr/bin/python

import declarations
import results
import matplotlib.pyplot as plt
import numpy as np

benchmark_labels = declarations.benchmark_labels
isolate_benchmark_path = declarations.isolate_benchmark_path 
process_benchmark_path = declarations.process_benchmark_path 
openwhisk_benchmark_path = declarations.openwhisk_benchmark_path 
photons_benchmark_path = declarations.photons_benchmark_path
snapshot_benchmark_path = declarations.snapshot_benchmark_path

# Memory footprint in GBs.
isolate_mem_avg = {}
isolate_mem_std = {}
process_mem_avg = {}
process_mem_std = {}
openwhisk_mem_avg = {}
openwhisk_mem_std = {}
photons_mem_avg = {}
photons_mem_std = {}
snapshot_mem_avg = {}
snapshot_mem_std = {}

# Processing results.
for idx, path in enumerate(isolate_benchmark_path):
    isolate_mem_avg[idx] = results.process_result(path)["mem_avg"]
    isolate_mem_std[idx] = results.process_result(path)["mem_std"]
for idx, path in enumerate(process_benchmark_path):
    process_mem_avg[idx] = results.process_result(path)["mem_avg"]
    process_mem_std[idx] = results.process_result(path)["mem_std"]
for idx, path in enumerate(openwhisk_benchmark_path):
    openwhisk_mem_avg[idx] = results.process_result(path)["mem_avg"]
    openwhisk_mem_std[idx] = results.process_result(path)["mem_std"]
for idx, path in enumerate(photons_benchmark_path):
    photons_mem_avg[idx] = results.process_result(path)["mem_avg"]
    photons_mem_std[idx] = results.process_result(path)["mem_std"]
for idx, path in enumerate(snapshot_benchmark_path):
    snapshot_mem_avg[idx] = results.process_result(path)["mem_avg"]
    snapshot_mem_std[idx] = results.process_result(path)["mem_std"]

# Scale openwhisk and snapshot to 2GB.
for idx, path in enumerate(openwhisk_benchmark_path):
    openwhisk_mem_avg[idx] = openwhisk_mem_avg[idx] * (2048 / declarations.openwhisk_mem_factor[idx])
for idx, path in enumerate(snapshot_benchmark_path):
    snapshot_mem_avg[idx] = snapshot_mem_avg[idx] * (2048 / declarations.snapshot_mem_factor[idx])

x = np.arange(len(benchmark_labels))
# Snapshot and openwhisk have special x values to avoid a gap when photons doesn't have a value.
snapshot_x  = np.arange(len(benchmark_labels), dtype=np.float32)
openwhisk_x = np.arange(len(benchmark_labels), dtype=np.float32)
width = 0.15

# Filling Photons values with zeroes for missing (java) benchmarks.
# We also adapt the x values for snapshot and openwhisk for avoid a gap when photons has no value.
for idx, path in enumerate(isolate_benchmark_path):
    if idx not in photons_mem_avg:
        photons_mem_avg[idx] = 0
        photons_mem_std[idx] = 0
        snapshot_x[idx]  -= width
        openwhisk_x[idx] -= width

plt.rcParams.update({'font.size': 12})
fig, ax = plt.subplots()
ax.bar(x + 0*width,           isolate_mem_avg.values(),   yerr=isolate_mem_std.values(),   width=width, hatch='*', label='Graalvisor', alpha=0.75)
ax.bar(x + 1*width,           process_mem_avg.values(),   yerr=process_mem_std.values(),   width=width, hatch='.', label='Forking',  alpha=0.75)
ax.bar(x + 2*width,           photons_mem_avg.values(),   yerr=photons_mem_std.values(),   width=width, hatch='O', label='Photons',    alpha=0.75)
ax.bar(snapshot_x  + 3*width, snapshot_mem_avg.values(),  yerr=snapshot_mem_std.values(),  width=width, hatch='/', label='VM Snapshot', alpha=0.75)
ax.bar(openwhisk_x + 4*width, openwhisk_mem_avg.values(), yerr=openwhisk_mem_std.values(), width=width, hatch='-', label='OpenWhisk',  alpha=0.75)

ax.set_ylabel('Memory (GBs)')
#ax.set_xticks(x, benchmark_labels)
plt.tick_params(
    axis='x',          # changes apply to the x-axis
    which='both',      # both major and minor ticks are affected
    bottom=False,      # ticks along the bottom edge are off
    top=False,         # ticks along the top edge are off
    labelbottom=False) # labels along the bottom edge are off
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
ax.set_ylim(ymax=2.1)
ax.set_xlim(xmin=-.25, xmax=14.7)
fig.set_figwidth(15)
fig.set_figheight(3)

#ax.legend(ncol=5, loc='upper center')
plt.savefig("sandboxes-memory.pdf", bbox_inches='tight')
plt.savefig("sandboxes-memory.png", bbox_inches='tight')
plt.show()
