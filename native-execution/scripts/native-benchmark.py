#!/usr/bin/python3

import argparse
import subprocess

num_transitions = 0

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

def parse_total_time(line):
    return int(line.split(" ")[4])

def parse_native_time(line):
    return int(line.split(" ")[2]) // 1000

def get_percentages(total_times, native_code_times):
    percentages = []
    for idx in range(len(total_times)):
        native_time_total = sum(native_code_times[idx])
        percentages.append((native_time_total / total_times[idx]) * 100)
    return percentages

def run(command, times):
    global num_transitions

    byte_output = subprocess.check_output(command + " " + str(times), shell=True)

    # Decode output and split into lines
    output_lines = byte_output.decode().strip().split("\n")

    idx = 0
    total_times = []
    native_code_times = [[] for i in range(times)]
    # Filter lines containing 'timer' or 'Total execution time'
    for line in output_lines:
        if 'Total execution time' in line:
            total_times.append(parse_total_time(line))
            idx += 1
        elif 'timer' in line:
            native_code_times[idx].append(parse_native_time(line))

    num_transitions = len(native_code_times[1])
    percentages = get_percentages(total_times, native_code_times)

    return total_times, percentages

def main():
    command, times = parse_args()

    total_times, percentages = run(command, times)

    avg_total_time = sum(total_times[1:]) / (times - 1)
    avg_native_percentage = sum(percentages[1:]) / (times - 1)

    print("Average percentage of native execution: {:.2f}".format(avg_native_percentage))
    print(f"Total number of transitions per invocation: {num_transitions}")
    print("Average function invocation time: {:.3f}".format(avg_total_time)) # time in us


if __name__ == "__main__":
    main()
