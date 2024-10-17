#!/usr/bin/python3

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


def run(command):
    start_time = time.perf_counter()
    byte_output = subprocess.check_output(command, shell=True)
    end_time = time.perf_counter()

    # Decode output and split into lines
    output_lines = byte_output.decode().split("\n")

    # Filter lines containing 'timer'
    timer_lines = [line for line in output_lines if 'timer' in line]

    # Extract the line with "Actual/total" and compute the untrusted percentage
    actual_total_lines = [s for s in output_lines if "Actual/total" in s]
    actual_count, total_count = map(float, actual_total_lines[-1].split()[-1].split("/"))

    # Calculate elapsed time in microseconds
    elapsed_time_microseconds = (end_time - start_time) * 1_000_000

    return timer_lines, elapsed_time_microseconds, (actual_count, total_count)

def main():
    command, times = parse_args()

    results = [(run(command)) for _ in range(times)]

    native_percentages = [process_results(result[0], result[1]) for result in results]
    total_times = [result[1] for result in results]
    actual_count = [result[2][0] for result in results]
    total_count = [result[2][1] for result in results]

    avg_total_time = np.sum(total_times) / times
    avg_native_percentage = np.sum(native_percentages) / times
    avg_actual_count = np.sum(actual_count) / times
    avg_total_count = np.sum(total_count) / times

    print("Average percentage of native execution: {:.2f}".format(avg_native_percentage))
    print(f"Number of transitions per second: {len(results[0][0])/(avg_total_time/1000000)}") # number of transitions is fixed
    print("Average untrusted native calls percentage: {:.2f}".format(avg_actual_count/avg_total_count))
    print(f"Actual/Total: {avg_actual_count}/{avg_total_count}")


if __name__ == "__main__":
    main()
