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

# Efficiency in ops/s/gb.
isolate_eff_avg = {}
isolate_eff_std = {}
process_eff_avg = {}
process_eff_std = {}
openwhisk_eff_avg = {}
openwhisk_eff_std = {}
photons_eff_avg = {}
photons_eff_std = {}
snapshot_eff_avg = {}
snapshot_eff_std = {}

# Processing results.
for idx, path in enumerate(isolate_benchmark_path):
    isolate_eff_avg[idx] = results.process_result(path)["eff_avg"]
    isolate_eff_std[idx] = results.process_result(path)["eff_std"]
for idx, path in enumerate(process_benchmark_path):
    process_eff_avg[idx] = results.process_result(path)["eff_avg"]
    process_eff_std[idx] = results.process_result(path)["eff_std"]
for idx, path in enumerate(openwhisk_benchmark_path):
    openwhisk_eff_avg[idx] = results.process_result(path)["eff_avg"]
    openwhisk_eff_std[idx] = results.process_result(path)["eff_std"]
for idx, path in enumerate(photons_benchmark_path):
    photons_eff_avg[idx] = results.process_result(path)["eff_avg"]
    photons_eff_std[idx] = results.process_result(path)["eff_std"]
for idx, path in enumerate(snapshot_benchmark_path):
    snapshot_eff_avg[idx] = results.process_result(path)["eff_avg"]
    snapshot_eff_std[idx] = results.process_result(path)["eff_std"]

x = np.arange(len(benchmark_labels))
# Snapshot and openwhisk have special x values to avoid a gap when photons doesn't have a value.
snapshot_x  = np.arange(len(benchmark_labels), dtype=np.float32)
openwhisk_x = np.arange(len(benchmark_labels), dtype=np.float32)
width = 0.15

# Normalization to isolate.
for idx, path in enumerate(process_benchmark_path):
    process_eff_avg[idx] = process_eff_avg[idx] / isolate_eff_avg[idx]
    process_eff_std[idx] = process_eff_std[idx] / isolate_eff_avg[idx]
for idx, path in enumerate(openwhisk_benchmark_path):
    openwhisk_eff_avg[idx] = openwhisk_eff_avg[idx] / isolate_eff_avg[idx]
    openwhisk_eff_std[idx] = openwhisk_eff_std[idx] / isolate_eff_avg[idx]
for idx, path in enumerate(photons_benchmark_path):
    photons_eff_avg[idx] = photons_eff_avg[idx] / isolate_eff_avg[idx]
    photons_eff_std[idx] = photons_eff_std[idx] / isolate_eff_avg[idx]
for idx, path in enumerate(snapshot_benchmark_path):
    snapshot_eff_avg[idx] = snapshot_eff_avg[idx] / isolate_eff_avg[idx]
    snapshot_eff_std[idx] = snapshot_eff_std[idx] / isolate_eff_avg[idx]
for idx, path in enumerate(isolate_benchmark_path):
    isolate_eff_std[idx] = isolate_eff_std[idx] / isolate_eff_avg[idx]
    isolate_eff_avg[idx] = 1
    # Filling Photons values with zeroes for missing (java) benchmarks.
    if idx not in photons_eff_avg:
        photons_eff_avg[idx] = 0
        photons_eff_std[idx] = 0
        snapshot_x[idx]  -= width
        openwhisk_x[idx] -= width

plt.rcParams.update({'font.size': 16})
fig, ax = plt.subplots()
ax.bar(x + 0*width,           isolate_eff_avg.values(),   yerr=isolate_eff_std.values(),   width=width, hatch='*', label='Graalvisor',  alpha=0.75)
ax.bar(x + 1*width,           process_eff_avg.values(),   yerr=process_eff_std.values(),   width=width, hatch='.', label='Forking',     alpha=0.75)
ax.bar(x + 2*width,           photons_eff_avg.values(),   yerr=photons_eff_std.values(),   width=width, hatch='O', label='Photons',     alpha=0.75)
ax.bar(snapshot_x  + 3*width, snapshot_eff_avg.values(),  yerr=snapshot_eff_std.values(),  width=width, hatch='/', label='VM Snapshot', alpha=0.75)
ax.bar(openwhisk_x + 4*width, openwhisk_eff_avg.values(), yerr=openwhisk_eff_std.values(), width=width, hatch='-', label='OpenWhisk',   alpha=0.75)

ax.set_ylabel('Efficiency norm. to Isolate')
ax.set_xticks(x, benchmark_labels)
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
ax.set_xlim(xmin=-.25, xmax=14.7)
fig.set_figwidth(15)
fig.set_figheight(4)

ax.legend(ncol=5, loc='upper center')
plt.savefig("sandboxes-efficiency-norm.pdf", bbox_inches='tight')
plt.savefig("sandboxes-efficiency-norm.png", bbox_inches='tight')
plt.show()
