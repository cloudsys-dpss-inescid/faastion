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

# Throughput in ops/s.
isolate_tput_avg = {}
isolate_tput_std = {}
process_tput_avg = {}
process_tput_std = {}
openwhisk_tput_avg = {}
openwhisk_tput_std = {}
photons_tput_avg = {}
photons_tput_std = {}
snapshot_tput_avg = {}
snapshot_tput_std = {}

# Processing results.
for idx, path in enumerate(isolate_benchmark_path):
    isolate_tput_avg[idx] = results.process_result(path)["tput_avg"]
    isolate_tput_std[idx] = results.process_result(path)["tput_std"]
for idx, path in enumerate(process_benchmark_path):
    process_tput_avg[idx] = results.process_result(path)["tput_avg"]
    process_tput_std[idx] = results.process_result(path)["tput_std"]
for idx, path in enumerate(openwhisk_benchmark_path):
    openwhisk_tput_avg[idx] = results.process_result(path)["tput_avg"]
    openwhisk_tput_std[idx] = results.process_result(path)["tput_std"]
for idx, path in enumerate(photons_benchmark_path):
    photons_tput_avg[idx] = results.process_result(path)["tput_avg"]
    photons_tput_std[idx] = results.process_result(path)["tput_std"]
for idx, path in enumerate(snapshot_benchmark_path):
    snapshot_tput_avg[idx] = results.process_result(path)["tput_avg"]
    snapshot_tput_std[idx] = results.process_result(path)["tput_std"]

# Scale openwhisk to 2GB.
for idx, path in enumerate(openwhisk_benchmark_path):
    openwhisk_tput_avg[idx] = openwhisk_tput_avg[idx] * (2048 / declarations.openwhisk_mem_factor[idx])
for idx, path in enumerate(snapshot_benchmark_path):
    snapshot_tput_avg[idx] = snapshot_tput_avg[idx] * (2048 / declarations.snapshot_mem_factor[idx])

# Normalization to isolate.
for idx, path in enumerate(process_benchmark_path):
        process_tput_avg[idx] = process_tput_avg[idx] / isolate_tput_avg[idx]
        process_tput_std[idx] = process_tput_std[idx] / isolate_tput_avg[idx]
for idx, path in enumerate(openwhisk_benchmark_path):
        openwhisk_tput_avg[idx] = openwhisk_tput_avg[idx] / isolate_tput_avg[idx]
        openwhisk_tput_std[idx] = openwhisk_tput_std[idx] / isolate_tput_avg[idx]
for idx, path in enumerate(photons_benchmark_path):
    photons_tput_avg[idx] = photons_tput_avg[idx] / isolate_tput_avg[idx]
    photons_tput_std[idx] = photons_tput_std[idx] / isolate_tput_avg[idx]
for idx, path in enumerate(snapshot_benchmark_path):
    snapshot_tput_avg[idx] = snapshot_tput_avg[idx] / isolate_tput_avg[idx]
    snapshot_tput_std[idx] = snapshot_tput_std[idx] / isolate_tput_avg[idx]
for idx, path in enumerate(isolate_benchmark_path):
    isolate_tput_std[idx] = isolate_tput_std[idx] / isolate_tput_avg[idx]
    isolate_tput_avg[idx] = 1
    # Filling Photons values with zeroes for missing (java) benchmarks.
    if idx not in photons_tput_avg:
        photons_tput_avg[idx] = 0
        photons_tput_std[idx] = 0

x = np.arange(len(benchmark_labels))
width = 0.15

plt.rcParams.update({'font.size': 16})

fig, ax = plt.subplots()
ax.bar(x + 1*width, isolate_tput_avg.values(),   yerr=isolate_tput_std.values(),   width=width, hatch='*', label='Graalvisor', alpha=0.75)
ax.bar(x + 2*width, process_tput_avg.values(),   yerr=process_tput_std.values(),   width=width, hatch='.',  label='Forking',  alpha=0.75)
ax.bar(x + 3*width, photons_tput_avg.values(),   yerr=photons_tput_std.values(),   width=width, hatch='O',  label='Photons',    alpha=0.75)
ax.bar(x + 4*width, snapshot_tput_avg.values(),   yerr=snapshot_tput_std.values(),   width=width, hatch='+',  label='VM Snapshot',    alpha=0.75)
ax.bar(x + 5*width, openwhisk_tput_avg.values(), yerr=openwhisk_tput_std.values(), width=width, hatch='-', label='OpenWhisk',   alpha=0.75)

ax.set_ylabel('Tput norm. to Isolate')
ax.set_xticks(x, benchmark_labels)
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
fig.set_figwidth(15)
fig.set_figheight(4)

ax.legend(ncol=5)
plt.savefig("sandboxes-tput-norm.pdf", bbox_inches='tight')
plt.savefig("sandboxes-tput-norm.png", bbox_inches='tight')
plt.show()
