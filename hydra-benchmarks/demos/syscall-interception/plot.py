#!/usr/bin/python
import os
import sys
import pathlib
from datetime import datetime
import matplotlib.pyplot as plt

class demo:
    bench_names = ["post", "get"]
    def __init__(self, suite):
        self.bench_avg = {}
        self.overheads = {}
        self.label = suite
        self.load_log_files(suite)

    def load_log_files(self, suite):
        log_files = list(filter(lambda x: ".log" in x, os.listdir(suite)))
        self.bench_avg = {os.path.splitext(log_file)[0]:self.read_log_file(os.path.join(suite, log_file)) 
                          for log_file in log_files}

    def read_log_file(self, log_file):
        try:
            f = open(log_file, 'r')
            data = f.read().split('\n')
            row = data[22]
            time_per_request = float(row.split()[3]) 
            f.close()
            return time_per_request
        except IOError as e:
            print(e)

    def calculate_overheads(self, native_demo):
        for bench in demo.bench_names:
            self.overheads[bench] = self.bench_avg[bench] / native_demo.bench_avg[bench]

def get_native_demo(demos):
    for demo in demos:
        if demo.label == "native":
            return demo
    raise Exception("native demo not found")

def calculate_overheads(demos):
    native_demo = get_native_demo(demos)
    for demo in demos:
        demo.calculate_overheads(native_demo)

def plot(demos): 
    calculate_overheads(demos)
    plt.rcParams["figure.figsize"] = (6,3)
    plt.rcParams.update({'font.size': 12})
    plt.rcParams.update({'figure.autolayout': True})
    plt.style.use('ggplot')
    fig = plt.figure()
    plt.tight_layout()
    plt.xticks(rotation = 45)
    plt.xlabel("Time (ms)")

    for bench in demo.bench_names:
        plt.clf()
        x = [i for i, _ in enumerate(demos)]
        demos_list = sorted(demos, key=lambda demo: demo.bench_avg[bench])
        bench_avg = [demo.bench_avg[bench] for demo in demos_list]
        labels = [demo.label for demo in demos_list]
        bars = plt.barh(x, bench_avg, color='green')
        plt.title("Time per " + bench.upper() + " Request")
        plt.yticks(x, labels)
        overheads = [demo.overheads[bench] for demo in demos_list]
        for i, v in enumerate(overheads):
            plt.text(bench_avg[i] + 0.04, i, str(round(v,2)), color='blue')
        plt.draw()
        output = datetime.now().strftime(bench.upper() + "_%Y%m%d_%H_%M_%S.jpg")
        plt.savefig(os.path.join("plots", output))

if __name__ == "__main__":
    try:
        pathlib.Path('plots').mkdir()
    except FileExistsError:
        pass
    
    def get_demos(to_search):
        return [demo(suite) for suite in to_search]
    
    # Run 
    to_search = sys.argv[1:] if len(sys.argv) > 1 else ['native', 'seccomp', 'seccomp_allow_rw', 'strace']
    demos = get_demos(to_search)
    plot(demos)
