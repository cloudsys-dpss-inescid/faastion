#!/bin/python3

import sys
import os
import re
import shutil
import pathlib
import statistics
import csv

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

def flatten(l):
    return [item for sublist in l for item in sublist]

def subtract_times(times):
    return [t[1] - t[0] for t in times]

def make_csv(csv_output_path, tag, jvm_send, jvm_recv, svm_send, svm_recv):
    mkd(csv_output_path)
    save_path = os.path.join(csv_output_path, f'{tag}.csv')
    try:
        with open(save_path, 'w') as f:
            f.write('jvm_send,jvm_recv,svm_send,svm_recv\n')
            for i in range(len(jvm_send)):
                f.write(f'{jvm_send[i]},{jvm_recv[i]},{svm_send[i]},{svm_recv[i]}\n')
    except IOError:
        print("I/O error")

def process_dir(dir):
    print('processing: ' + dir)
    dir_basename = os.path.basename(os.path.normpath(dir))
    csv_output_path = os.path.join('csv', dir_basename)

    for test_dir in os.listdir(dir):
        test_dir_path = os.path.join(dir, test_dir)
        print('processing: ' + test_dir_path)
        jvm_send = subtract_times(read_log_file(os.path.join(test_dir_path, 'jvm_client.log')))
        jvm_recv = subtract_times(read_log_file(os.path.join(test_dir_path, 'jvm_server.log')))
        svm_send = subtract_times(read_log_file(os.path.join(test_dir_path, 'svm_client.log')))
        svm_recv = subtract_times(read_log_file(os.path.join(test_dir_path, 'svm_server.log')))
        make_csv(csv_output_path, test_dir, jvm_send, jvm_recv, svm_send, svm_recv)


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


