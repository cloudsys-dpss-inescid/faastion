#!/bin/python3

import sys
import os
import re
import shutil
import pathlib
import statistics
import numpy as np
from matplotlib import pyplot as plt

timestamp_regex = re.compile(r'^\[(?P<ts>[0-9]+)\]')

def mkd(path):
    try:
        pathlib.Path(path).mkdir()
    except FileExistsError:
        pass

def extract_timestamp(line):
    m = timestamp_regex.match(line)
    if m is None:
        raise Exception("failed to parse timestamp from line `" + line + "`")
    return int(m.group("ts"))

def read_log_file(path):
    try:
        with open(path, 'r') as f:
            contents = f.readlines()

        matches = ['sending #', 'sent #', 'receiving #', 'received #']
        times = []
        time = []
        for line in contents:
            if not any(map(lambda m: m in line, matches)):
                continue
            t = extract_timestamp(line)
            time.append(t)
            if len(time) == 2:
                times.append(time)
                time = []
        return times
    except IOError as e:
        print(e)
        sys.exit(1)

def subtract_times(times):
    return [t[1] - t[0] for t in times]

def total_travel_time(client_times, server_times):
    return [t[1][1] - t[0][0] for t in zip(client_times, server_times)]

def cutoff(times):
    n = len(times)
    cutoff = int(n / 10)
    return sorted(times)[cutoff:-cutoff]

def extract_mean(client_log, server_log):
    assert len(client_log) == len(server_log), "inconsistent sample sizes"

    send_times = subtract_times(client_log)
    recv_times = subtract_times(server_log)
    comp_times = total_travel_time(client_log, server_log)

    comp_times = cutoff(comp_times)

    return [statistics.mean(comp_times), np.std(np.array(comp_times))]
    # return [statistics.mean(send_times), np.std(np.array(send_times))]
    # return [statistics.mean(recv_times), np.std(np.array(recv_times))]

def plot(title, data, plot_output_path):
    labels = sorted(data.keys())
    jvm_means = []
    svm_means = []
    for label in labels:
        jvm_means.append(round(data[label][0][0]/1000, 0))
        svm_means.append(round(data[label][1][0]/1000, 0))

    fig, ax = plt.subplots()

    x = np.arange(len(labels))
    width = 0.45  # the width of the bars
    rects1 = ax.bar(x - width/2, jvm_means, width, label='JVM')
    rects2 = ax.bar(x + width/2, svm_means, width, label='SVM')

    ax.set_ylabel('t (us)')
    ax.set_title(title)
    ax.set_xticks(x, labels)
    ax.legend()

    ax.bar_label(rects1, padding=3)
    ax.bar_label(rects2, padding=3)

    fig.tight_layout()

    mkd(plot_output_path)
    save_path = os.path.join(plot_output_path, 'plot.png')
    print("saving plot to", save_path)
    plt.savefig(save_path)

def plot_one(title, data, plot_output_path, tag):
    # print(title, tag, data[:10])
    # print(title, tag, data[-10:])

    y = cutoff(data)[::250]
    labels = range(1, len(y) + 1)

    fig, ax = plt.subplots()

    x = np.arange(len(labels))
    width = 0.2  # the width of the bars
    rects1 = ax.bar(x - width/2, y, width)

    ax.set_ylabel('t (us)')    
    ax.set_title(f'{title}_{tag}')

    fig.tight_layout()

    mkd(plot_output_path)
    save_path = os.path.join(plot_output_path, f'plot_{tag}.png')
    print("saving plot to", save_path)
    plt.savefig(save_path)
    plt.close(fig)


def plot_dir(dir):
    print('processing: ' + dir)
    dir_basename = os.path.basename(os.path.normpath(dir))
    plot_output_path = os.path.join('plots', dir_basename)

    data = {}
    for test_dir in os.listdir(dir):
        test_dir_path = os.path.join(dir, test_dir)
        print('processing: ' + test_dir_path)
        jvm_client_log = read_log_file(os.path.join(test_dir_path, 'jvm_client.log'))
        jvm_server_log = read_log_file(os.path.join(test_dir_path, 'jvm_server.log'))
        svm_client_log = read_log_file(os.path.join(test_dir_path, 'svm_client.log'))
        svm_server_log = read_log_file(os.path.join(test_dir_path, 'svm_server.log'))
        plot_one(dir_basename, subtract_times(jvm_client_log), plot_output_path, f'{test_dir}_jvm_send')
        plot_one(dir_basename, subtract_times(jvm_server_log), plot_output_path, f'{test_dir}_jvm_recv')
        plot_one(dir_basename, subtract_times(svm_client_log), plot_output_path, f'{test_dir}_svm_send')
        plot_one(dir_basename, subtract_times(svm_server_log), plot_output_path, f'{test_dir}_svm_recv')
        plot_one(dir_basename, total_travel_time(jvm_client_log, jvm_server_log), plot_output_path, f'{test_dir}_jvm_total')
        plot_one(dir_basename, total_travel_time(svm_client_log, svm_server_log), plot_output_path, f'{test_dir}_svm_total')
        jvm_mean = extract_mean(jvm_client_log, jvm_server_log)
        svm_mean = extract_mean(svm_client_log, svm_server_log)
        data[test_dir] = (jvm_mean, svm_mean)

    plot(dir_basename, data, plot_output_path)

if __name__ == "__main__":

    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} log_dir")
        sys.exit(1)
    
    try:
        shutil.rmtree('plots')
    except IOError:
        pass
    mkd('plots')

    for dir in sys.argv[1:]:
        plot_dir(dir)

