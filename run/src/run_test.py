#!/usr/bin/python3
import os
import re
import shlex
import subprocess
import sys
import threading
import time

import requests

from run_utils import GENERAL_INFO, write_json_file, random_tmp_filename, remove_tmp_files
from run_utils import MessageTypeWUser as MessageType
from run_utils import print_message_ as print_message
from run_utils import read_file_ as read_file
from run_utils import read_json_file_ as read_json_file
from run_utils import write_file_ as write_file
from run_globals import MAX_VERBOSITY_LVL, BENCHMARK_BUILD_SCRIPT, LAMBDA_MANAGER_LOG_FILE

# Global variables.
VERBOSITY_LVL = 0


def set_verbosity(flag):
    global VERBOSITY_LVL
    v_count = flag.count("v")
    if v_count != len(flag):
        print_message(GENERAL_INFO, "Verbosity flag should be v or vv instead of {flag}. Output verbosity level will "
                                    "fallback to 0."
                      .format(flag=flag), MessageType.WARN)
        return
    VERBOSITY_LVL = min(v_count, MAX_VERBOSITY_LVL)
    print_message(GENERAL_INFO, "Output verbosity level is set to {level}.".format(level=VERBOSITY_LVL),
                  MessageType.SPEC)


# Core methods.
def run(username, command):
    outs, errs = subprocess.Popen(shlex.split(command),
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    outs, errs = outs.decode(sys.stdout.encoding), errs.decode(sys.stdout.encoding)
    if len(outs) > 0 and VERBOSITY_LVL > 1:
        print_message(username, outs, MessageType.NO_HEADER)
    if len(errs) > 0 and VERBOSITY_LVL > 0:
        print_message(username, "Command ({}) error log:\n{}".format(command, errs), MessageType.WARN)
    return outs


def configure_managers(test_config_dir, managers):
    for manager in managers:
        print_message(GENERAL_INFO, "Response: " +
                      requests.post("{manager}/configure_manager".format(manager=manager['address']),
                                    headers={'Content-type': 'application/json'},
                                    data=read_file(GENERAL_INFO,
                                                   os.path.join(test_config_dir, manager['config_path']))).text,
                      MessageType.INFO)


def register_managers(load_balancer, managers):
    for manager in managers:
        if not manager['register_in_load_balancer']:
            continue
        print_message(GENERAL_INFO, "Response: " +
                      requests.post("{load_balancer}/register_manager?upstream=manager_cluster&add=&server={manager}"
                                    .format(load_balancer=load_balancer, manager=manager)).text, MessageType.INFO)


def build_benchmark(username, source):
    benchmark_path = None
    if os.path.isdir(source):
        # If source is directory, find build script and build benchmark.
        print_message(username, "Building benchmark...", MessageType.INFO)
        output = run(username,
                     "bash {source_path}/{script_name}".format(source_path=os.path.abspath(source),
                                                               script_name=BENCHMARK_BUILD_SCRIPT))
        try:
            benchmark_path = re.search('BENCHMARK_PATH=(.+)', output).group(1)
        except AttributeError:
            print_message(username, "BENCHMARK_PATH variable is not specified!", MessageType.ERROR)
            exit(2)
        print_message(username, "Building benchmark...done", MessageType.INFO)
    else:
        # If source is file (like python, javascript script etc.), skip build step.
        benchmark_path = source
    return benchmark_path


def upload_function(test_config_dir, username, entry_point, command_info):
    benchmark_path = build_benchmark(username, os.path.join(test_config_dir, command_info['source']))
    print_message(username, "Response: " +
                  requests.post("{entry_point}/upload_function?"
                                "username={username}&"
                                "function_name={function_name}&"
                                "function_language={function_language}&"
                                "function_entry_point={function_entry_point}&"
                                "function_memory={function_memory}&"
                                "function_runtime={function_runtime}"
                                .format(entry_point=entry_point,
                                        username=username,
                                        function_name=command_info['function_name'],
                                        function_language=command_info['function_language'],
                                        function_entry_point=command_info['function_entry_point'],
                                        function_memory=command_info['function_memory'],
                                        function_runtime=command_info['function_runtime']),
                                headers={'Content-type': 'application/octet-stream'},
                                data=read_file(username, benchmark_path)).text, MessageType.INFO)


def remove_function(username, entry_point, command_info):
    print_message(username, "Response: " + requests.post("{entry_point}/remove_function?"
                                                         "username={username}&"
                                                         "function_name={function_name}"
                                                         .format(entry_point=entry_point,
                                                                 username=username,
                                                                 function_name=command_info['function_name'])).text,
                  MessageType.INFO)


def pause(username, command_info):
    print_message(username, "Pausing...", MessageType.INFO)
    time.sleep(command_info['duration'])
    print_message(username, "Pausing...done", MessageType.INFO)


def send(test_config_dir, username, manager, command_info):
    real_output_path = os.path.join(test_config_dir, command_info['output'])
    path = os.path.dirname(real_output_path)
    if len(path) > 0:
        os.makedirs(path, exist_ok=True)
    if os.path.exists(real_output_path):
        os.remove(real_output_path)

    for i in range(command_info['iterations']):
        print_message(username, "Iteration {} of {}...".format(i, command_info['iterations'] - 1), MessageType.INFO)
        output = "ITERATION({})...\n".format(i)

        tmp_file = random_tmp_filename()
        write_json_file(tmp_file, command_info['parameters'])
        output += run(username,
                      "ab -p {file_w_parameters} -n {num_requests} -c {num_clients} -T application/json "
                      "{manager}/{username}/{function_name}"
                      .format(file_w_parameters=tmp_file,
                              num_requests=command_info['num_requests'],
                              num_clients=command_info['num_clients'],
                              manager=manager,
                              username=username,
                              function_name=command_info['function_name']
                              ))
        output += "ITERATION({})...done\n\n".format(i)
        write_file(username, real_output_path, output)
        remove_tmp_files([tmp_file])

        print_message(username, "Iteration {} of {}...done".format(i, command_info['iterations'] - 1), MessageType.INFO)


def start_sending(test_config_dir, username, manager, command_info):
    send_threads = []
    for send_info in command_info['sending_info']:
        send_thread = threading.Thread(target=send, args=(test_config_dir, username, manager, send_info))
        send_thread.start()
        send_threads.append(send_thread)

    for send_thread in send_threads:
        send_thread.join()


def check_failure_pattern(username, failure_pattern):
    lambda_manager_log = read_file(username, LAMBDA_MANAGER_LOG_FILE, 'r')
    if len(lambda_manager_log) == 0:
        print_message(username, "Lambda manager log is empty!", MessageType.WARN)
        return
    regex = re.compile(failure_pattern)
    found_failure_patterns = regex.findall(lambda_manager_log)
    if len(found_failure_patterns) > 0:
        print_message(username, "Failure pattern is found {} time(s)!".format(len(found_failure_patterns)),
                      MessageType.WARN)
    else:
        print_message(username, "Success pattern is found!", MessageType.SPEC)


def create_user(test_config_dir, user_info, manager):
    print_message(user_info['username'], "{} is sending commands...".format(user_info['username']), MessageType.INFO)

    kindness_counter = 0
    for command_info in user_info['commands']:
        command_info['command'].lower()
        if command_info['command'] == "u" or command_info['command'] == "upload":
            upload_function(test_config_dir, user_info['username'], manager, command_info)
            continue
        if command_info['command'] == "r" or command_info['command'] == "remove":
            remove_function(user_info['username'], manager, command_info)
            continue
        if command_info['command'] == "s" or command_info['command'] == "send":
            start_sending(test_config_dir, user_info['username'], manager, command_info)
            continue
        if command_info['command'] == "p" or command_info['command'] == "pause":
            pause(user_info['username'], command_info)
            continue
        if command_info['command'] == "k" or command_info['command'] == "kindness":
            kindness_counter += 1
    if kindness_counter > 0:
        print_message(user_info['username'], "Thank you, {}, for your kindness!".format(user_info['username']),
                      MessageType.SPEC)

    check_failure_pattern(user_info['username'], user_info['failure_pattern'])

    print_message(user_info['username'], "{} is sending commands...done".format(user_info['username']),
                  MessageType.INFO)


def unregister_managers(load_balancer, managers):
    for manager in managers:
        if not manager['register_in_load_balancer']:
            continue
        print_message(GENERAL_INFO, "Response: " +
                      requests.post("{load_balancer}/register_manager?upstream=manager_cluster&remove=&server={manager}"
                                    .format(load_balancer=load_balancer, manager=manager)).text, MessageType.INFO)


def test(test_config_dir, data):
    print_message(GENERAL_INFO, "Test - {} - is running...".format(data['test']), MessageType.INFO)

    register_managers(data['entry_point'], data['managers'])
    configure_managers(test_config_dir, data['managers'])

    users = []
    for user_info in data['users']:
        user = threading.Thread(target=create_user, args=(test_config_dir, user_info, data['entry_point']))
        user.start()
        users.append(user)

    for user in users:
        user.join()

    unregister_managers(data['entry_point'], data['managers'])

    print_message(GENERAL_INFO, "Test - {} - is running...done".format(data['test']), MessageType.INFO)


# Main function.
def main(args):
    if len(args) == 0:
        print_message(GENERAL_INFO, "Insufficient number of arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)
    test_config_index = 0
    if len(args) == 1:
        test_config_index = 0
        print_message(GENERAL_INFO, "Output verbosity level will be 0.", MessageType.SPEC)
    elif len(args) == 2:
        test_config_index = 1
        set_verbosity(args[0])
    else:
        print_message(GENERAL_INFO, "Too much arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)

    test(os.path.dirname(args[test_config_index]), read_json_file(args[test_config_index]))


if __name__ == '__main__':
    main(sys.argv[1:])
