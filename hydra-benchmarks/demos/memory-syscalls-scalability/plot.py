#!/bin/python

import os
import sys
import pathlib
from statistics import mean

from matplotlib import pyplot as plt
import numpy as np


def plot(suite, log_files):
    print("processing suite", suite, "with log files:", log_files)

    data = {}
    for log_file in log_files:
        log_file_parent_dir = os.path.dirname(log_file)
        output_dir = os.path.join("plots", log_file_parent_dir)
        pathlib.Path(output_dir).mkdir(parents=True, exist_ok=True)
        log_file_basename = os.path.splitext(os.path.basename(log_file))[0]
        output_file = os.path.join(output_dir, log_file_basename)
    
        data[log_file_basename] = read_log_file(log_file)

        output_single = output_file + '.jpg'
        print("plotting:", log_file, "to", output_single)
        pyplot_one(data[log_file_basename], output_single)

    sanity_check(suite, data)

    output_all = os.path.join(output_dir, suite + '.jpg')
    print("plotting:", suite, "summary to", output_all)
    pyplot_all(data, output_all)


def pyplot_one(y, output_file):
    plt.clf()

    n = len(y)
    x = range(1, n+1)
    
    plt.title(output_file)
    plt.ylabel("t (s)")
    plt.tick_params(bottom=False)
    plt.xticks(rotation=30, ha='center')

    plt.bar(x, y)

    plt.savefig(output_file)
    

def pyplot_all(data, output_file):
    plt.clf()

    labels = data.keys()
    avg_times = list(map(lambda l: mean(data[l]), labels))
    std_dev = list(map(lambda l: np.std(data[l]), labels))
    n = len(labels)

    SMALL_SIZE = 12
    MEDIUM_SIZE = 15
    BIGGER_SIZE = 20

    plt.rc('font', size=MEDIUM_SIZE)         # controls default text sizes
    plt.rc('axes', titlesize=BIGGER_SIZE)    # fontsize of the axes title
    plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
    plt.rc('xtick', labelsize=MEDIUM_SIZE)   # fontsize of the tick labels
    plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
    plt.rc('legend', fontsize=MEDIUM_SIZE)   # legend fontsize
    plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title

    # colors = ['#a3ceb1', '#ebebd3', '#e8d3b6', '#a3aec0', '#e0bcc6'][:n]
    plt.figure(figsize=(10,14))

    plt.title(suite)
    plt.ylabel("t (s)")
    plt.tick_params(bottom=False)
    plt.xticks(rotation=30, ha='center')

    plt.bar(labels, avg_times)
    # plt.bar(labels, avg_times, width=0.8, color=colors)
    # plt.errorbar(labels, avg_times, yerr=std_dev, fmt='.', color='black')
    for x, y in zip(labels, avg_times):
        label = "{:g}".format(y)
        plt.annotate(label,
                    (x,y/2),
                    textcoords="offset points",
                    xytext=(0,0),
                    ha='center')

    plt.savefig(output_file)


def read_log_file(log_file):
    try:
        with open(log_file, 'r') as f:
            lines = f.readlines()
            lines_to_parse = list(filter(lambda line: line and line[0] != '[', lines))
            return [float(f) for f in lines_to_parse]
    except IOError as e:
        print(e)

def sanity_check(suite, data):
    sizes = [len(v) for k, v in data.items()]
    assert len(set(sizes)) == 1, "different sample sizes found for suite: " + suite


if __name__ == '__main__':

    try:
        import shutil
        shutil.rmtree('plots')
        pathlib.Path('plots').mkdir()
        pathlib.Path('plots/.testignore').touch()
    except FileNotFoundError:
        pass

    def discover_log_files(suite):
        if os.path.isdir(suite):
            suite = suite.rstrip("/")
            ls = os.listdir(suite)
            if '.testignore' in ls:
                return
            for filename in ls:
                if filename.endswith(".log"):
                    if not suite in data:
                        data[suite] = []
                    data[suite].append(os.path.join(suite, filename))

    data = {}

    to_search = sys.argv[1:] if len(sys.argv) > 1 else os.listdir(os.curdir)
    for suite in to_search:
        discover_log_files(suite)
            
    for suite, log_files in data.items():
        plot(suite, log_files)