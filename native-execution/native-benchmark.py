#!/usr/bin/python

import argparse
import numpy as np
import subprocess
import time

from multiprocessing import Manager, cpu_count
from concurrent.futures import ProcessPoolExecutor


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-c", 
        "--command", 
        type=str, 
        dest="command", 
        help="Javassist command"
    )
    parser.add_argument(
        "-t",
        "--times",
        type=int,
        dest="times",
        default=5,
        help="Times to run (Returns average)",
    )

    args = parser.parse_args()

    return args.command.strip(), args.times


def native_execution_us(values):
    my_array = np.array(list(values))
    return np.sum(my_array) / 1000  # nanoseconds to microseconds


def build_table(line, table):
    if "timer" not in line: return

    splitted = line.split(" ")
    number = int(splitted[2])
    method = splitted[0]

    if method in table:
        table[method] += number
    else:
        table[method] = number


def process_results(dump, total_time):
    with Manager() as manager:
        table = manager.dict()

        with ProcessPoolExecutor(max_workers=cpu_count()) as executor:
            executor.map(build_table, dump, [table] * len(dump))

        native_time = native_execution_us(table.values())
    
    return (native_time * 100) / total_time


def run(cmd):
    start_time = time.perf_counter()
    b_output = subprocess.check_output(cmd, shell=True)
    end_time = time.perf_counter()

    output = b_output.decode().split("\n")            # -> array with lines as elements
    dump = [s for s in output if 'timer' in s]        # -> filter output

    return dump, (end_time - start_time) * 1_000_000


def main():
    command, times = parse_args()

    results = [(run(command)) for _ in range(times)]

    native_percentages = [process_results(result[0], result[1]) for result in results]
    total_times = [result[1] for result in results]

    avg_total_time = np.sum(total_times) / times
    avg_native_percentage = np.sum(native_percentages) / times

    print("Average percentage of native execution: {:.2f}".format(avg_native_percentage))
    print(f"Number of transitions per second: {len(results[0][0])/(avg_total_time/1000000)}") # number of transitions is fixed


if __name__ == "__main__":
    main()
