#!/usr/bin/python

import results
import matplotlib.pyplot as plt
import numpy as np

benchmark_labels = [
    "jv/hw",
#    "java/sleep",
    "jv/hashing",
    "jv/rest",
    "jv/video",
    "jv/classify"
]

isolate_benchmark_path = [
    "java/gv-hello-world-niuk-isolate-benchmark-8-1-2048",
#    "java/gv-sleep-niuk-benchmark-10-1-2048",
    "java/gv-file-hashing-niuk-isolate-benchmark-8-1-2048",
    "java/gv-httprequest-niuk-isolate-benchmark-8-1-2048",
    "java/gv-video-processing-niuk-isolate-benchmark-2-1-2048",
    "java/gv-classify-niuk-isolate-benchmark-1-1-2048",
]

runtime_benchmark_path = [
    "java/gv-hello-world-niuk-runtime-benchmark-8-1-2048",
#    "java/gv-sleep-niuk-benchmark-10-1-2048",
    "java/gv-file-hashing-niuk-runtime-benchmark-8-1-2048",
    "java/gv-httprequest-niuk-runtime-benchmark-8-1-2048",
    "java/gv-video-processing-niuk-runtime-benchmark-2-1-2048",
    "java/gv-classify-niuk-runtime-benchmark-1-1-2048",
]

process_benchmark_path = [
    "java/gv-hello-world-niuk-process-benchmark-8-1-2048",
#    "java/gv-sleep-niuk-benchmark-10-1-2048",
    "java/gv-file-hashing-niuk-process-benchmark-8-1-2048",
    "java/gv-httprequest-niuk-process-benchmark-8-1-2048",
    "java/gv-video-processing-niuk-process-benchmark-2-1-2048",
    "java/gv-classify-niuk-process-benchmark-1-1-2048",
]

# Efficiency in ops/s/gb.
isolate_eff_avg = {}
isolate_eff_std = {}
runtime_eff_avg = {}
runtime_eff_std = {}
process_eff_avg = {}
process_eff_std = {}

for path in isolate_benchmark_path:
    isolate_eff_avg[path] = results.process_result(path)["eff_avg"]
    isolate_eff_std[path] = results.process_result(path)["eff_std"]
for path in runtime_benchmark_path:
    runtime_eff_avg[path] = results.process_result(path)["eff_avg"]
    runtime_eff_std[path] = results.process_result(path)["eff_std"]
for path in process_benchmark_path:
    process_eff_avg[path] = results.process_result(path)["eff_avg"]
    process_eff_std[path] = results.process_result(path)["eff_std"]

x = np.arange(len(benchmark_labels))
width = 0.15

fig, ax = plt.subplots()
ax.bar(x - width, isolate_eff_avg.values(), yerr=isolate_eff_std.values(), width=width, hatch='//', label='Isolate', alpha=0.75)
ax.bar(x        , runtime_eff_avg.values(), yerr=runtime_eff_std.values(), width=width, hatch='..', label='Runtime', alpha=0.75)
ax.bar(x + width, process_eff_avg.values(), yerr=process_eff_std.values(), width=width, hatch='.', label='Process', alpha=0.75)

ax.set_ylabel('Tput/Mem (Ops/Second/GB)')
ax.set_yscale('log')
ax.set_ylim(ymin=0.1)
ax.set_xticks(x, benchmark_labels)
ax.legend()
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
fig.set_figwidth(10)
fig.set_figheight(3)
plt.savefig("sandboxes-efficiency.pdf", bbox_inches='tight')
plt.show()
