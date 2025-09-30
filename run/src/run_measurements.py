#!/usr/bin/python3
import os
import re
import sys
import time
from datetime import datetime

from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

from run_utils import MessageType, print_message, read_file, read_file_unsafe, read_json_file
from run_globals import LAMBDA_MANAGER_LOG_FILE, LAMBDA_MANAGER_DIR

# Global variables.
INFLUXDB_URL = ""
INFLUXDB_AUTH_TOKEN = ""
INFLUXDB_ORG = ""
INFLUXDB_BUCKET = ""
SLEEP_TIME_SEC = 10

MANAGER_LOG_LATENCY_REGEX = "Timestamp \\((\\d+)\\) FINE Time " \
                            "\\(user=(.*),function_name=(.*),state=(.*),id=(\\d+)\\):\\s*(\\d+)\\s*\\[ms\\]"
MANAGER_LOG_EXIT_RECORD_REGEX = "Timestamp \\((\\d+)\\).*PID -> (\\d+).*--output=(.*)\\/memory\\.log.*Exit"
BOOT_TIME_REGEX = "VMM boot time: (\\d+)"

LAMBDA_TABLE = {}


# Core methods.
def process_latency(most_recent_timestamp, write_api):
    matches = fetch_request_latencies(most_recent_timestamp)
    for entry in matches:
        update_lambda_table(pid=entry[4], username=entry[1], function_name=entry[2])
        write_request_latency(write_api=write_api, username=entry[1], function_name=entry[2],
                              lambda_execution_mode=entry[3], request_latency_value=entry[5])
        most_recent_timestamp = entry[0]
    return most_recent_timestamp


def fetch_request_latencies(most_recent_timestamp):
    file = read_file_unsafe(LAMBDA_MANAGER_LOG_FILE)
    # to start matching from most recent timestamp
    index = file.rfind(f'Timestamp ({most_recent_timestamp})')
    index = 0 if index == -1 else index + 1
    regex = re.compile(MANAGER_LOG_LATENCY_REGEX)
    return regex.findall(file[index:])


def write_request_latency(write_api, username, function_name, lambda_execution_mode, request_latency_value):
    print(
        f'Latency: sent data point: {username} {function_name} {lambda_execution_mode}, {request_latency_value}')
    point = (Point("request_latency")
             .tag("execution_mode", lambda_execution_mode)
             .tag("user", username)
             .tag("function", function_name)
             .field("latency", int(request_latency_value))
             .time(datetime.utcnow(), WritePrecision.NS))
    write_api.write(INFLUXDB_BUCKET, INFLUXDB_ORG, point)


def process_startup(most_recent_timestamp, write_api):
    matches = fetch_output_logs(most_recent_timestamp)
    for entry in matches:
        output_log_filename = os.path.join(LAMBDA_MANAGER_DIR, entry[2], "output.log")
        if entry[1] in LAMBDA_TABLE:
            info = LAMBDA_TABLE[entry[1]]  # get lambda info by PID
            write_startup(write_api=write_api, filename=output_log_filename,
                          username=info["username"], function_name=info["function_name"])
        most_recent_timestamp = entry[0]
    return most_recent_timestamp


def fetch_output_logs(most_recent_timestamp):
    file = read_file_unsafe(LAMBDA_MANAGER_LOG_FILE)
    # to start matching from most recent timestamp
    index = file.rfind(f'Timestamp ({most_recent_timestamp})')
    index = 0 if index == -1 else index + 1
    regex = re.compile(MANAGER_LOG_EXIT_RECORD_REGEX)
    return regex.findall(file[index:])


def write_startup(write_api, filename, username, function_name):
    file = read_file(filename)
    regex = re.compile(BOOT_TIME_REGEX)
    matches = regex.findall(file)
    if matches:
        print(f'Startup: sent data point: {matches[0]}')
        point = (Point("startup_time")
                 .tag("user", username)
                 .tag("function", function_name)
                 .field("startup", int(matches[0]))
                 .time(datetime.utcnow(), WritePrecision.NS))
        write_api.write(INFLUXDB_BUCKET, INFLUXDB_ORG, point)


def monitor():
    most_recent_timestamp_latency = -1
    most_recent_timestamp_startup = -1
    client = InfluxDBClient(url=INFLUXDB_URL, token=INFLUXDB_AUTH_TOKEN)
    write_api = client.write_api(write_options=SYNCHRONOUS)
    while True:
        try:
            most_recent_timestamp_latency = process_latency(
                most_recent_timestamp_latency, write_api)
            most_recent_timestamp_startup = process_startup(
                most_recent_timestamp_startup, write_api)
        except IOError:
            print_message("Manager log file is missing or deleted! Retrying in {} sec".format(
                SLEEP_TIME_SEC), MessageType.WARN)
        time.sleep(SLEEP_TIME_SEC)


def update_lambda_table(pid, username, function_name):
    if pid not in LAMBDA_TABLE:
        LAMBDA_TABLE[pid] = {"username": username,
                             "function_name": function_name}


def init_configuration(config):
    global INFLUXDB_URL
    global INFLUXDB_AUTH_TOKEN
    global INFLUXDB_ORG
    global INFLUXDB_BUCKET
    global SLEEP_TIME_SEC
    INFLUXDB_URL = config['influxdb_url']
    INFLUXDB_AUTH_TOKEN = config['influxdb_token']
    INFLUXDB_ORG = config['influxdb_org']
    INFLUXDB_BUCKET = config['influxdb_bucket']
    SLEEP_TIME_SEC = config['interval']


# Main function.
def main(args):
    if len(args) == 0:
        print_message("Insufficient number of arguments ({})!".format(len(args)), MessageType.ERROR)
        exit(1)
    if len(args) == 1:
        init_configuration(read_json_file(args[0]))
    else:
        print_message("Too much arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)

    monitor()


if __name__ == '__main__':
    main(sys.argv[1:])
