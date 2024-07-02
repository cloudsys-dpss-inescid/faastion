#!/usr/bin/python

import os
import numpy as np

def read_benchmark_throughput(path):
    tput = []
    try:
        with open(path + '/ab.log') as file:
            for line in file:
                if 'Requests per second:' in line:
                    tput.append(float(line.split()[3]))
    except Exception as e:
        print("Error processing " + path + ":")
        raise e
    return tput

def read_benchmark_memory(path):
    mem = []
    try:
        mem = []
        with open(path + '/lambda.rss') as file:
            for line in file:
                mem.append(int(line))
        if len(mem) < 7:
            print("Warning, not enough memory data points from {path}".format(path=path))
        # Use the last 5 samples before the last one.
        mem = mem[-6:-1]
    except Exception as e:
        print("Error processing " + path + ":")
        raise e
    return mem

def process_result(path):
    # Throughput (ops/s) of each iteration.
    tput = []
    # Memory samples (rss in kbs) of each iteration.
    mem = []

    for file in os.scandir('../results/' + path):
        if file.is_dir():
            mem.extend(read_benchmark_memory(file.path))
            tput.extend(read_benchmark_throughput(file.path))

    # Convert to numpy arrays (help with mean and std).
    tput = np.array(tput)
    mem = np.array(mem)

    # Convertion from kbs to mbs and to gbs.
    mem = mem / 1024 / 1024

    # Numpy array with the efficiency (ops/s/GB) samples.
    eff = tput / mem.mean()
    results = {}
    results["tput_avg"] = round(tput.mean(), 3)
    results["tput_std"] = round(tput.std(), 3)
    results["mem_avg"] = round(mem.mean(), 3)
    results["mem_std"] = round(mem.std(), 3)
    results["eff_avg"] = round(eff.mean(), 3)
    results["eff_std"] = round(eff.std(), 3)
    print("{path} {results}".format(path=path, results=results))
    return results
