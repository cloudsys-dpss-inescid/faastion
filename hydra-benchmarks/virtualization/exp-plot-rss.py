#!/usr/bin/python3

import matplotlib.pyplot as plt
import statistics

bench_lbl = []
bench_avg = []
bench_std = []

def load_data(path, label):
  try:
    with open(path, 'r') as f:
      data_kbs = [float(line.rstrip()) for line in f]
      data_mbs = [value/1000 for value in data_kbs]      
      bench_avg.append(statistics.mean(data_mbs))
      bench_lbl.append(label)
  except FileNotFoundError:
    print('File not found:' + path)

load_data('results/rss-isolate.dat',     'NI Isolate')
load_data('results/rss-gv-isolate.dat',  'GV Sandbox')
load_data('results/rss-gv-fork.dat',     'GV Sandbox (forked)')
load_data('results/rss-ni.dat',          'Native Image')
load_data('results/rss-python.dat',      'CPython')
load_data('results/rss-node.dat',        'NodeJS')
load_data('results/rss-hotspot.dat',     'JVM')
load_data('results/rss-firecracker/rss-firecracker.dat', 'Firecracker')
load_data('results/rss-qemu/rss-qemu.dat',           'Qemu')
load_data('results/rss-docker.dat',              'Docker (graalvm)')
load_data('results/rss-docker-scratch.dat',      'Docker (scratch)')

plt.rcParams.update({'figure.autolayout': True})
plt.style.use('ggplot')

x = [i for i, _ in enumerate(bench_lbl)]

fig = plt.figure()
plt.tight_layout()
bars = plt.barh(x, bench_avg, color='green')
#bars[3].set_color('red')
plt.xticks(rotation = 45)
plt.xlabel("RSS (MBs)")
plt.title("Memory Footprint of Virtualization Technologies")
plt.yticks(x, bench_lbl)

for i, v in enumerate(bench_avg):
  plt.text(v + 4, i, str(round(v,2)), color='blue')

fig.savefig("results/rss.png")

