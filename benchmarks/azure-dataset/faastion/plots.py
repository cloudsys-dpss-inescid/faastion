import matplotlib.pyplot as plt
import numpy as np

#static mpk e dynamic mpk (faastion)
APPROACHES = ["faastion", "faastlane", "isolate", "process"]

def get_axis_values(approach: str, metric: str):
    metrics = []
    times = []
    
    with open(f"/tmp/{approach}/{metric}.csv", mode="r") as file:
        lines = file.readlines()
        timestamp = int(lines[0].split(",")[1])

        for line in lines:
            splitted = line.split(",")
            if metric in ["footprint", "execution"]:
                metrics.append(int(splitted[0])/1000) # (MB) or (ms)
                times.append((int(splitted[1]) - timestamp)/1000) # (ms) to (s)
            elif metric == "latency":
                metrics.append(int(splitted[0])/1_000_000) # (ms)
                times.append((int(splitted[1]) - timestamp)/1000) # (ms) to (s)
            else: # domains
                metrics.append(int(splitted[0]))
                times.append((int(splitted[1]) - timestamp)/1_000_000) # (us) to (s)

    return np.array(times), np.array(metrics)


def main():
    fig, axs = plt.subplots(2, 2, figsize=(14, 8))

    # Flatten the axs array for easier indexing
    axs = axs.flatten()

    # Latency CDF graph
    for approach in APPROACHES:
        _, latency = get_axis_values(approach, "latency")
        sorted_latency = np.sort(latency)
        cumulative_prob = np.arange(len(sorted_latency)) / float(len(sorted_latency))
        axs[0].plot(sorted_latency, cumulative_prob, label=approach)
    axs[0].set_ylabel("Cumulative Probability")
    axs[0].set_xlabel("Latency (ms)")
    axs[0].legend(loc='upper right')

    for approach in APPROACHES:
        _, execution = get_axis_values(approach, "execution")
        sorted_execution = np.sort(execution)
        cumulative_prob = np.arange(len(sorted_execution)) / float(len(sorted_execution))
        axs[1].plot(sorted_execution, cumulative_prob, label=approach)
    axs[1].set_ylabel("Cumulative Probability")
    axs[1].set_xlabel("Process Time (ms)")
    axs[1].legend(loc='upper right')

    for approach in APPROACHES:
        x, y = get_axis_values(approach, "footprint")
        axs[2].plot(x, y, label=approach)
    axs[2].set_ylabel("Memory Footprint (MB)")
    axs[2].set_xlabel("Time (s)")
    axs[2].legend(loc='upper left')

    # Domains/time graph
    for approach in ["faastion", "faastlane"]:
        x, y = get_axis_values(approach, "domains")
        axs[3].plot(x, y, label=approach)
    axs[3].set_ylabel("Domains usage")
    axs[3].set_xlabel('Time (s)')
    axs[3].legend(loc='upper right')
    axs[3].set_xlim(0, 600)

    plt.tight_layout()
    plt.savefig('plots.png')

if __name__ == "__main__":
    main()