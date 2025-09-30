#!/usr/bin/python3
import enum
import glob

import json
import os
import random
import string
from json import JSONDecodeError


# Message type.
class MessageType(enum.Enum):
    INFO = "[INFO]"
    ERROR = "[ERR0]"
    WARN = "[WARN]"
    SPEC = "[SPEC]"
    NO_HEADER = ""


# Message type with specified user.
class MessageTypeWUser(enum.Enum):
    INFO = "[INFO-{}]"
    ERROR = "[ERR0-{}]"
    WARN = "[WARN-{}]"
    SPEC = "[SPEC-{}]"
    NO_HEADER = ""


# Plot type.
class PlotType(enum.Enum):
    LATENCY = "latency"
    THROUGHPUT = "throughput"
    STARTUP = "startup"
    FOOTPRINT = "footprint"
    SCALABILITY = "scalability"
    TOTAL_MEMORY = "total memory"
    CDF = "cdf"


# Script type.
class ScriptType(enum.Enum):
    TEST = "test"
    PLOT = "plot"
    INSTALL_DEPS = "install-deps"
    TESTS = "tests"
    MEASUREMENTS = "measurements"
    MONITORING = "monitoring"
    BUILD = "build"
    DEPLOY = "deploy"
    HELP = "help"


class ExecutionType(enum.Enum):
    HOTSPOT_W_AGENT = "Hotspot w agent"
    HOTSPOT = "Hotspot"
    VMM = "VMM"


GENERAL_INFO = "general"
FILENAME_LENGTH = 10


# Print functions.
def print_message(message, t):
    if t == MessageType.INFO:
        print('\033[1;39m' + t.value + " " + message + '\033[0m')  # default
        return
    if t == MessageType.ERROR:
        print('\033[1;31m' + t.value + " " + message + '\033[0m')  # red
        return
    if t == MessageType.WARN:
        print('\033[1;33m' + t.value + " " + message + '\033[0m')  # yellow
        return
    if t == MessageType.SPEC:
        print('\033[1;32m' + t.value + " " + message + '\033[0m')  # green
        return
    print(message)


def print_message_(username, message, t):
    if t == MessageTypeWUser.INFO:
        print('\033[1;39m' + t.value.format(username) + " " + message + '\033[0m')  # default
        return
    if t == MessageTypeWUser.ERROR:
        print('\033[1;31m' + t.value.format(username) + " " + message + '\033[0m')  # red
        return
    if t == MessageTypeWUser.WARN:
        print('\033[1;33m' + t.value.format(username) + " " + message + '\033[0m')  # yellow
        return
    if t == MessageTypeWUser.SPEC:
        print('\033[1;32m' + t.value.format(username) + " " + message + '\033[0m')  # green
        return
    print(message)


# File utils.
def read_json_file(filename):
    try:
        with open(filename) as input_file:
            return json.load(input_file)
    except IOError:
        print_message("Input file {} is missing or deleted!".format(filename), MessageType.ERROR)
        exit(1)
    except JSONDecodeError as err:
        print_message("Bad JSON syntax: {}".format(err), MessageType.ERROR)
        exit(1)


def read_json_file_(filename):
    try:
        with open(filename) as input_file:
            return json.load(input_file)
    except IOError:
        print_message_(GENERAL_INFO, "Input file {} is missing or deleted!".format(filename), MessageTypeWUser.ERROR)
        exit(1)
    except JSONDecodeError as err:
        print_message_(GENERAL_INFO, "Bad JSON syntax: {}".format(err), MessageTypeWUser.ERROR)
        exit(1)


def write_json_file(filename, content):
    with open(filename, 'w') as outfile:
        json.dump(content, outfile)


def read_file(filename):
    try:
        with open(filename) as input_file:
            return input_file.read()
    except IOError:
        print_message("Input file {} is missing or deleted!".format(filename), MessageType.ERROR)
        exit(1)


def read_file_unsafe(filename):
    with open(filename) as input_file:
        return input_file.read()


def read_file_(username, filename, read_type='rb'):
    try:
        with open(filename, read_type) as input_file:
            return input_file.read()
    except IOError:
        print_message_(username, "Input file {} is missing or deleted!".format(filename), MessageTypeWUser.ERROR)
        exit(1)


def write_file(filename, content):
    try:
        with open(filename, 'w') as output_file:
            output_file.write(content)
    except IOError:
        print_message("Error during writing in output file {}!".format(filename), MessageType.ERROR)
        exit(1)


def write_file_(username, filename, content):
    try:
        with open(filename, 'a') as output_file:
            output_file.write(content)
    except IOError:
        print_message_(username, "Error during writing in output file {}!".format(filename), MessageTypeWUser.ERROR)
        exit(1)


def remove_file(filename):
    os.remove(filename)


def random_tmp_filename():
    return ''.join(random.choice(string.ascii_lowercase) for _ in range(FILENAME_LENGTH)) + ".tmp"


def remove_leftover_tmp_files():
    remove_tmp_files(glob.glob("*.tmp"))


def remove_tmp_files(files):
    for file in files:
        os.remove(file)


# Math utils.
def kb_to_mb(value):
    return value / 1000


def kb_to_gb(value):
    return value / 1_000_000
