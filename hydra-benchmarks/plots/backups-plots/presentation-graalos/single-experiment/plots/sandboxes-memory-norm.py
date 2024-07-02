#!/usr/bin/python

import results
import matplotlib.pyplot as plt
import numpy as np

benchmark_labels = [
    "jv/hw",
    "jv/hashing",
    "jv/rest",
    "jv/video",
    "jv/classify",
    "jv/shopcart",
]

isolate_benchmark_path = [
    "java/gv-hello-world-niuk-isolate-benchmark-1-1-2048",
    "java/gv-file-hashing-niuk-isolate-benchmark-1-1-2048",
    "java/gv-httprequest-niuk-isolate-benchmark-1-1-2048",
    "java/gv-video-processing-niuk-isolate-benchmark-1-1-2048",
    "java/gv-classify-niuk-isolate-benchmark-1-1-2048",
    "java/gv-shopcart-niuk-isolate-benchmark-1-1-2048",
]

runtime_benchmark_path = [
    "java/gv-hello-world-niuk-runtime-benchmark-1-1-2048",
    "java/gv-file-hashing-niuk-runtime-benchmark-1-1-2048",
    "java/gv-httprequest-niuk-runtime-benchmark-1-1-2048",
    "java/gv-video-processing-niuk-runtime-benchmark-1-1-2048",
    "java/gv-classify-niuk-runtime-benchmark-1-1-2048",
    "java/gv-shopcart-niuk-runtime-benchmark-1-1-2048",
]

process_benchmark_path = [
    "java/gv-hello-world-niuk-process-benchmark-1-1-2048",
    "java/gv-file-hashing-niuk-process-benchmark-1-1-2048",
    "java/gv-httprequest-niuk-process-benchmark-1-1-2048",
    "java/gv-video-processing-niuk-process-benchmark-1-1-2048",
    "java/gv-classify-niuk-process-benchmark-1-1-2048",
    "java/gv-shopcart-niuk-process-benchmark-1-1-2048",
]

# Memory footprint in GBs.
isolate_mem_avg = {}
isolate_mem_std = {}
runtime_mem_avg = {}
runtime_mem_std = {}
process_mem_avg = {}
process_mem_std = {}

for path in isolate_benchmark_path:
    isolate_mem_avg[path] = results.process_result(path)["mem_avg"]
    isolate_mem_std[path] = results.process_result(path)["mem_std"]
for path in runtime_benchmark_path:
    runtime_mem_avg[path] = results.process_result(path)["mem_avg"]
    runtime_mem_std[path] = results.process_result(path)["mem_std"]
for path in process_benchmark_path:
    process_mem_avg[path] = results.process_result(path)["mem_avg"]
    process_mem_std[path] = results.process_result(path)["mem_std"]

# Normalization to isolate.
for path in process_benchmark_path:
    process_mem_avg[path] = process_mem_avg[path] / isolate_mem_avg[path.replace("-process-", "-isolate-")]
    process_mem_std[path] = process_mem_std[path] / isolate_mem_avg[path.replace("-process-", "-isolate-")]
for path in runtime_benchmark_path:
    runtime_mem_avg[path] = runtime_mem_avg[path] / isolate_mem_avg[path.replace("-runtime-", "-isolate-")]
    runtime_mem_std[path] = runtime_mem_std[path] / isolate_mem_avg[path.replace("-runtime-", "-isolate-")]
for path in isolate_benchmark_path:
    isolate_mem_std[path] = isolate_mem_std[path] / isolate_mem_avg[path]
    isolate_mem_avg[path] = 1

x = np.arange(len(benchmark_labels))
width = 0.15

plt.rcParams.update({'font.size': 16})

fig, ax = plt.subplots()
ax.bar(x - width, isolate_mem_avg.values(), yerr=isolate_mem_std.values(), width=width, hatch='//', label='Isolate', alpha=0.75)
ax.bar(x        , runtime_mem_avg.values(), yerr=runtime_mem_std.values(), width=width, hatch='..', label='Runtime', alpha=0.75)
ax.bar(x + width, process_mem_avg.values(), yerr=process_mem_std.values(), width=width, hatch='.', label='Process', alpha=0.75)

ax.set_ylim([.9, 1.1])
ax.set_ylabel('Memory norm. to Isolate')
ax.set_xticks(x, benchmark_labels)
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
fig.set_figwidth(10)
fig.set_figheight(4)

ax.legend(ncol=3, loc='upper center')
plt.savefig("sandboxes-memory-norm.pdf", bbox_inches='tight')
plt.savefig("sandboxes-memory-norm.png", bbox_inches='tight')
plt.show()
