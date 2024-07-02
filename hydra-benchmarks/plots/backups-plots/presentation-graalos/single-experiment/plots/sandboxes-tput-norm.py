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
    "jv/shopcart"
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

# Throughput in ops/s.
isolate_tput_avg = {}
isolate_tput_std = {}
runtime_tput_avg = {}
runtime_tput_std = {}
process_tput_avg = {}
process_tput_std = {}

for path in isolate_benchmark_path:
    isolate_tput_avg[path] = results.process_result(path)["tput_avg"]
    isolate_tput_std[path] = results.process_result(path)["tput_std"]
for path in runtime_benchmark_path:
    runtime_tput_avg[path] = results.process_result(path)["tput_avg"]
    runtime_tput_std[path] = results.process_result(path)["tput_std"]
for path in process_benchmark_path:
    process_tput_avg[path] = results.process_result(path)["tput_avg"]
    process_tput_std[path] = results.process_result(path)["tput_std"]

# Normalization to isolate.
for path in process_benchmark_path:
    process_tput_avg[path] = process_tput_avg[path] / isolate_tput_avg[path.replace("-process-", "-isolate-")]
    process_tput_std[path] = process_tput_std[path] / isolate_tput_avg[path.replace("-process-", "-isolate-")]
for path in runtime_benchmark_path:
    runtime_tput_avg[path] = runtime_tput_avg[path] / isolate_tput_avg[path.replace("-runtime-", "-isolate-")]
    runtime_tput_std[path] = runtime_tput_std[path] / isolate_tput_avg[path.replace("-runtime-", "-isolate-")]
for path in isolate_benchmark_path:
    isolate_tput_std[path] = isolate_tput_std[path] / isolate_tput_avg[path]
    isolate_tput_avg[path] = 1

x = np.arange(len(benchmark_labels))
width = 0.15

plt.rcParams.update({'font.size': 16})

fig, ax = plt.subplots()
ax.bar(x - width, isolate_tput_avg.values(), yerr=isolate_tput_std.values(), width=width, hatch='//', label='Isolate', alpha=0.75)
ax.bar(x        , runtime_tput_avg.values(), yerr=runtime_tput_std.values(), width=width, hatch='..', label='Runtime', alpha=0.75)
ax.bar(x + width, process_tput_avg.values(), yerr=process_tput_std.values(), width=width, hatch='.', label='Process', alpha=0.75)

ax.set_ylim([.9, 1.1])
ax.set_ylabel('Tput norm. to Isolate')
ax.set_xticks(x, benchmark_labels)
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
fig.set_figwidth(10)
fig.set_figheight(4)

ax.legend(ncol=3, loc='upper center')
plt.savefig("sandboxes-tput-norm.pdf", bbox_inches='tight')
plt.savefig("sandboxes-tput-norm.png", bbox_inches='tight')
plt.show()
plt.show()
