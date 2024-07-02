#!/bin/python3

import sys
import os
import re
import shutil
import pathlib
import statistics
import csv

def mkd(path):
    try:
        pathlib.Path(path).mkdir()
    except FileExistsError:
        pass

def extract_timestamp(line):
    return int(line)

def read_log_file(path):
    try:
        with open(path, 'r') as f:
            contents = f.readlines()

        times = []
        for line in contents:
            if line.startswith('['):
                continue
            t = extract_timestamp(line)
            times.append(t)
        return times
    except IOError as e:
        print(e)
        sys.exit(1)

def flatten(l):
    return [item for sublist in l for item in sublist]

def make_csv(csv_output_path, tag, jvm_log, svm_log):
    mkd(csv_output_path)
    save_path = os.path.join(csv_output_path, f'{tag}.csv')
    try:
        with open(save_path, 'w') as f:
            f.write('jvm_log,svm_log\n')
            for i in range(len(jvm_log)):
                f.write(f'{jvm_log[i]},{svm_log[i]}\n')
    except IOError:
        print("I/O error")

def process_dir(dir):
    print('processing: ' + dir)
    dir_basename = os.path.basename(os.path.normpath(dir))
    csv_output_path = os.path.join('csv', dir_basename)

    for test_dir in os.listdir(dir):
        test_dir_path = os.path.join(dir, test_dir)
        print('processing: ' + test_dir_path)
        jvm_log = read_log_file(os.path.join(test_dir_path, 'jvm.log'))
        svm_log = read_log_file(os.path.join(test_dir_path, 'svm.log'))
        make_csv(csv_output_path, test_dir, jvm_log, svm_log)


if __name__ == "__main__":

    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} log_dir")
        sys.exit(1)
    
    try:
        shutil.rmtree('csv')
    except IOError:
        pass
    mkd('csv')

    for dir in sys.argv[1:]:
        process_dir(dir)


