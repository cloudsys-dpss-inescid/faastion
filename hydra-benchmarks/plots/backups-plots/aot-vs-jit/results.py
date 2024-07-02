#!/usr/bin/python

import json
import os
import numpy as np


# Read bench_results.json file and extract array of queries.
def read_benchmark_results(path):
    with open(path) as bench_file:
        bench_data = json.load(bench_file)
        return bench_data["queries"]


# Read all config files into an array of JSONs.
def read_config_files(path):
    result = []
    for file in os.scandir(path):
        if file.is_file():
            with open(file) as config_file:
                result.append(json.load(config_file))
    return result


def get_benchmark_metric(metric_name, bench_results):
    metric_object = list(filter(lambda o: o["metric.name"] == metric_name, bench_results))[0]
    return metric_object["metric.value"]


def get_startup_metric(bench_results):
    startups = list(filter(lambda o: o["metric.name"] == "app-startup", bench_results))
    startups = list(x["metric.value"] for x in startups)
    print(startups)
    startups = np.array(startups)
    return round(startups.mean(), 3)


def read_build_output(path):
    result = {}
    try:
        with open(path) as build_log_file:
            for line in build_log_file:
                if 'Peak RSS:' in line:
                    tokens = line.split()
                    rss = tokens[11].split('GB')[0]
                    result["rss"] = float(rss)
                    result["cpu"] = float(tokens[15])
                elif 'Finished generating' in line:
                    tokens = line.split()
                    mins = int(tokens[4].split('m')[0])
                    seconds = int(tokens[5].split('s')[0])
                    result["time"] = mins * 60 + seconds
    except Exception as e:
        print("Error processing " + path + ":")
        raise e
    return result
