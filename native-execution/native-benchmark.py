import argparse
import numpy as np
import subprocess
import time

from multiprocessing import Manager, cpu_count
from concurrent.futures import ProcessPoolExecutor


CMD = "./javassist-cmd.sh {package} {entrypoint}"


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-p", 
        "--package", 
        type=str, 
        dest="package", 
        help="Package"
    )
    parser.add_argument(
        "-e", 
        "--entrypoint", 
        type=str, 
        dest="entrypoint", 
        help="Entrypoint"
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

    return args.package, args.entrypoint, args.times


def native_execution_us(values):
    my_array = np.array(list(values))
    return np.sum(my_array) / 1000  # nanoseconds to microseconds


def build_table(line, table):
    if "ns" not in line: return

    splitted = line.split(" ")
    number = int(splitted[2])
    method = splitted[0]

    if method in table:
        table[method] += number
    else:
        table[method] = number


def run(cmd, iter, last):
    start_time = time.perf_counter()
    b_output = subprocess.check_output(cmd, shell=True)
    end_time = time.perf_counter()
    total_time = (end_time - start_time) * 1_000_000  # seconds to microseconds

    output = b_output.decode().split("\n")            # -> array with lines as elements
    dump = [s for s in output if 'ns' in s]           # -> filter output

    with Manager() as manager:
        table = manager.dict()

        with ProcessPoolExecutor(max_workers=cpu_count()) as executor:
            executor.map(build_table, dump, [table] * len(dump))

        native_time = native_execution_us(table.values())
        native_percentage = (native_time * 100) / total_time

        if iter == last:
            print(f"Number of transitions: {len(dump)}")
        # print(f"Total time per method (ns): {table}")

    return native_percentage


def main():
    package, entrypoint, times = parse_args()
    native_percentages = []

    # Get Javassist command
    javassist_cmd = subprocess.check_output(
        CMD.format(package=package, entrypoint=entrypoint), shell=True
    ).decode()

    # Run benchmark
    for i in range(times):
        native_percentages.append(run(javassist_cmd, i, times-1))
    avg_native_percentage = np.sum(native_percentages) / times

    print(
        "Average percentage of native execution: {:.2f}%".format(avg_native_percentage)
    )


if __name__ == "__main__":
    main()
