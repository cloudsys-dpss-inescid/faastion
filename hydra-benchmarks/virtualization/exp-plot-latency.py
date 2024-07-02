#!/usr/bin/python3

import matplotlib.pyplot as plt
import statistics

bench_lbl = []
bench_avg = []
bench_std = []

def load_data(path, label):
  try:
    with open(path, 'r') as f:
      data_ms = [float(line.rstrip()) for line in f]
      bench_avg.append(statistics.mean(data_ms))
      bench_std.append(statistics.stdev(data_ms) if len(data_ms) > 1 else 0)
      bench_lbl.append(label)
  except FileNotFoundError:
    print('File not found:' + path)

load_data('results/latency-isolate.dat',    'NI Isolate')
load_data('results/latency-gv-isolate-median.dat', 'GV Sandbox')
load_data('results/latency-gv-fork-median.dat',    'GV Sandbox (forked)')
load_data('results/latency-ni.dat',         'Native Image')
load_data('results/latency-python.dat',     'CPython')
load_data('results/latency-node.dat',       'NodeJS')
load_data('results/latency-hotspot.dat',    'JVM')
load_data('results/latency-firecracker/latency-firecracker.dat', 'Firecracker')
load_data('results/latency-qemu/latency-qemu.dat',               'Qemu')
load_data('results/latency-docker-scratch.dat',                  'Docker Run (scratch)')
load_data('results/latency-docker.dat',                          'Docker Run (graalvm)')
load_data('results/latency-create-docker-scratch.dat',           'Docker Create (scratch)')
load_data('results/latency-create-docker.dat',                   'Docker Create (graalvm)')
load_data('results/latency-start-docker-scratch.dat',            'Docker Start (scratch)')
load_data('results/latency-start-docker.dat',                    'Docker Start (graalvm)')
load_data('results/latency-rm-docker-scratch.dat',               'Docker Remove (scratch)')
load_data('results/latency-rm-docker.dat',                       'Docker Remove (graalvm)')

plt.rcParams.update({'figure.autolayout': True})
plt.style.use('ggplot')

x = [i for i, _ in enumerate(bench_lbl)]

fig = plt.figure()
plt.tight_layout()
print(bench_avg)
bars = plt.barh(x, bench_avg, color='green')
plt.xticks(rotation = 45)
plt.xlabel("Time (ms)")
plt.title("Latency of Virtualization Technologies")
plt.yticks(x, bench_lbl)

for i, v in enumerate(bench_avg):
  plt.text(v + 4, i, str(round(v,2)), color='blue')

fig.savefig("results/latency.png")

