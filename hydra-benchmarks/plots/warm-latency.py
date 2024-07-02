#!/usr/bin/python

import os
import matplotlib.pyplot as plt
import numpy as np

results_home = "/home/rbruno/git/graalvm-argo-benchmarks/results"

benchmark_labels = [
    "jv/hw",
    "py/hw",
    "js/hw",
#    "java/sleep",
#    "python/sleep",
#    "javascript/sleep",
    "jv/hashing",
    "js/html",
    "py/html",
    "py/thumbnail",
    "js/uploader",
    "jv/rest",
    "jv/video",
    "py/uploader",
    "py/compress",
    "py/video",
    "jv/thumbnail",
    "jv/classify",
#    "py/mst"
]

gv_benchmark_path = [
    "java/gv-hello-world-niuk-test-100-1-2048",
    "python/gv-hello-world-niuk-test-100-1-2048",
    "javascript/gv-hello-world-niuk-test-100-1-2048",
#    "java/gv-sleep-niuk-test-10-1-2048",
#    "python/gv-sleep-niuk-test-10-1-2048",
#    "javascript/gv-sleep-niuk-test-10-1-2048",
    "java/gv-file-hashing-niuk-test-100-1-2048",
    "javascript/gv-dynamic-html-niuk-test-100-1-2048",
    "python/gv-dynamic-html-niuk-test-100-1-2048",
    "python/gv-thumbnail-niuk-test-100-1-2048",
    "javascript/gv-uploader-niuk-test-100-1-2048",
    "java/gv-httprequest-niuk-test-100-1-2048",
    "java/gv-video-processing-niuk-test-50-1-2048",
    "python/gv-uploader-niuk-test-100-1-2048",
    "python/gv-compression-niuk-test-100-1-2048",
    "python/gv-video-processing-niuk-test-50-1-2048",
    "javascript/gv-thumbnail-niuk-test-100-1-2048",
    "java/gv-classify-niuk-test-50-1-2048",
#    "python/gv-mst-niuk-test-100-1-2048"
]

cr_benchmark_path = [
    "java/cr-hello-world-test-100-1-2048",
    "python/cr-hello-world-test-100-1-2048",
    "javascript/cr-hello-world-test-100-1-2048",
#    "java/cr-sleep-test-10-1-2048",
#    "python/cr-sleep-test-10-1-2048", 
#    "javascript/cr-sleep-test-10-1-2048",
    "java/cr-file-hashing-test-100-1-2048",
    "javascript/cr-dynamic-html-test-100-1-2048",
    "python/cr-dynamic-html-test-100-1-2048",
    "python/cr-thumbnail-test-100-1-2048",
    "javascript/cr-uploader-test-100-1-2048",
    "java/cr-httprequest-test-100-1-2048",
    "java/cr-video-processing-test-50-1-2048",
    "python/cr-uploader-test-100-1-2048",
    "python/cr-compression-test-100-1-2048",
    "python/cr-video-processing-test-50-1-2048",
    "javascript/cr-thumbnail-test-100-1-2048",
    "java/cr-classify-test-50-1-2048",
#    "python/cr-mst-test-100-1-2048"
]

gv_benchmark_avg_latency = {}
cr_benchmark_avg_latency = {}

def read_benchmark_latency(path, values):
    try:
        benchmark_latency = []
        with open('../results/' + path + '/app.log') as file:
            for line in file:
                if 'Time taken:' in line:
                    # 1000 to scale from us to ms.
                    benchmark_latency.append(int(line.split()[2]) / 1000)
        last_elements = benchmark_latency[-25:]
        values[path] = int(sum(last_elements) / len(last_elements))
    except Exception as e:
        print("Error processing " + path + ":")
        raise e

for path in gv_benchmark_path: read_benchmark_latency(path, gv_benchmark_avg_latency)
for path in cr_benchmark_path: read_benchmark_latency(path, cr_benchmark_avg_latency)

print("########## Latency# ##########")
for key in gv_benchmark_avg_latency:
    print("{benchmark}: {value} ms".format(benchmark=key, value=gv_benchmark_avg_latency[key]))
for key in cr_benchmark_avg_latency:
    print("{benchmark}: {value} ms".format(benchmark=key, value=cr_benchmark_avg_latency[key]))

print("########## Latency Over. #####")
for i in range(len(benchmark_labels)):
    print("{benchmark}: {value} latency overhead w/ cr".format(benchmark=benchmark_labels[i], value=cr_benchmark_avg_latency[cr_benchmark_path[i]]/gv_benchmark_avg_latency[gv_benchmark_path[i]]))


x = np.arange(len(benchmark_labels))
width = 0.25

fig, ax = plt.subplots()
ax.bar(x - width/2, gv_benchmark_avg_latency.values(), width, hatch='//', label='Graalvisor', alpha=0.75 )
ax.bar(x + width/2, cr_benchmark_avg_latency.values(), width, hatch='..', label='OpenWisk', alpha=0.75)

ax.set_ylabel('Time (ms)')
ax.set_yscale('log')
ax.set_ylim(ymin=1)
ax.set_xticks(x, benchmark_labels)
ax.legend()
ax.set_axisbelow(True)
plt.grid(axis = 'y', linestyle = '--', linewidth = 0.25)
plt.xticks(rotation = 35)
fig.set_figwidth(10)
fig.set_figheight(3)
plt.savefig("warm-latency.pdf", bbox_inches='tight')
plt.show()
