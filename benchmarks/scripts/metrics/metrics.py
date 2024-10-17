import matplotlib.pyplot as plt
import numpy as np

APPROACHES = ["faastlane", "process", "faastion", "isolate"]
BENCHMARKS = ["hello-world", "native-hello-world", "matmul", "httrequest", "factorization", "sleep"]

COLORS = {
    "faastion": "gold",
    "faastlane": "orange",
    "process": "seagreen",
    "isolate": "blue"
}
LABELS = {
    "faastion": "Faastion",
    "faastlane": "Faastlane",
    "process": "Process-based",
    "isolate": "Memory Isolates"
}

def get_latency_values():
    approaches_latencies = {}

    for approach in APPROACHES:
        latencies = []
        stdevs = []
        
        for benchmark in ["hw", "nativehw", "matmul", "httprequest", "factors", "sleep"]:
            with open(f"/tmp/{approach}/{benchmark}-latency.log", mode="r") as file:
                lines = file.readlines()
                benchmark_latencies = [float(line.strip())/1000 for line in lines]
                latencies.append(np.mean(benchmark_latencies))
                stdevs.append(np.std(benchmark_latencies))
    
        approaches_latencies[approach] = np.array(latencies), np.array(stdevs)

    return approaches_latencies


def get_footprint_values(approach: str):
    footprints = []
    times = []
    
    with open(f"/tmp/{approach}/footprint.csv", mode="r") as file:
        lines = file.readlines()
        timestamp = int(lines[0].split(",")[1])

        for line in lines:
            splitted = line.split(",")
            footprints.append(int(splitted[0])/1000) # (MB)
            times.append((int(splitted[1]) - timestamp)/1000) # (seconds)

    return np.array(times), np.array(footprints)


def get_domain_values(approach: str):
    domains = []
    times = []
    
    with open(f"/tmp/{approach}/domains.csv", mode="r") as file:
        lines = file.readlines()
        timestamp = int(lines[0].split(",")[1])

        for line in lines:
            splitted = line.split(",")
            domains.append(int(splitted[0]))
            times.append((int(splitted[1]) - timestamp)/1_000_000) # (seconds)

    return np.array(times), np.array(domains)


# LATENCY
approaches_latencies = get_latency_values()

x = np.arange(len(BENCHMARKS))  # the label locations
width = 0.2  # the width of the bars
multiplier = 0

fig, ax = plt.subplots(layout='constrained')

for approach, measurements in approaches_latencies.items():
    latencies, stdevs = measurements
    offset = width * multiplier
    ax.bar(x + offset, latencies, width, color=COLORS[approach], label=LABELS[approach], yerr=stdevs)
    multiplier += 1

# Add some text for labels, title and custom x-axis tick labels, etc.
ax.set_ylabel('Server Side Latency (ms)')
ax.set_title('Average latency per approach')
ax.set_xticks([r + width for r in range(len(BENCHMARKS))], BENCHMARKS, rotation=15)

ax.legend(loc='upper left')
ax.set_ylim(bottom=0)

plt.savefig('latency.png')

# FOOTPRINT
fig, ax = plt.subplots(layout='constrained')

for approach in APPROACHES:
    x, y = get_footprint_values(approach)
    ax.plot(x, y, label=LABELS[approach], color=COLORS[approach])
ax.set_ylabel(f"Memory Footprint (MB)")
ax.set_xlabel("Time (s)")
ax.legend(loc='upper left')
ax.set_xlim(left=0)
ax.set_ylim(bottom=0)

plt.savefig('footprint.png')

# Domain usage/time graph
fig, ax = plt.subplots(layout='constrained')

for approach in ["faastion", "faastlane"]:
    x, y = get_domain_values(approach)
    ax.plot(x, y, label=LABELS[approach], color=COLORS[approach])
ax.set_ylabel(f"Domain usage")
ax.set_xlabel("Time (s)")
ax.legend(loc='upper left')
ax.set_xlim(left=0)
ax.set_ylim(bottom=0)

plt.savefig('domains.png')
