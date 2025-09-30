#!/usr/bin/python3
import os
import signal
import sys

import run_build
import run_deploy
import run_install_deps
import run_measurements
import run_monitoring
import run_plot
import run_test
import run_tests
from run_utils import MessageType, print_message, ScriptType, remove_leftover_tmp_files


def print_help_message():
    print('''
Argo's command-line tool.

The Run helps you with building, testing, plotting, installing dependencies...

    test [ v | vv ] <path to test configuration>
                      allow you to run test with specified verbosity level and test configuration.
    tests [ v | vv ] [ <comma-sep-filter-list> ]
                      allow you to run all tests for specific tiers and languages with selected verbosity.
    plot [ v | vv ] <path to plot configuration>
                      create plots based on configuration with given verbosity.
    install-deps [ v | vv ]
                      install all necessary dependencies.
    measurements <path to measurements configuration>
                      collect latency and startup time metrics.
    monitoring
                      run grafana, influxdb and monitoring server.
    build [ lb ] | [ lp ] | [ cm ] | [ lm ] | [ gv-lib ]
                      build load balancer, lambda proxy, cluster manager, lambda manager and graalvisor library (separately or together).
    deploy [ lb ] | [ cm ] | [ lm ]
                      deploy load balancer, cluster manager and lambda manager (separately or together).
    help              print help and exit.
    ''')


def print_error_message():
    print_message("Command not found. Try help?", MessageType.ERROR)


def cancel_execution_handler(sig, frame):
    print_message("Execution was interrupted with signal {} at frame {}!".format(sig, frame), MessageType.ERROR)
    remove_leftover_tmp_files()
    os._exit(2)


def parse_input_arguments(args):
    if args[0] == ScriptType.TEST.value:
        run_test.main(args[1:])
        return
    if args[0] == ScriptType.PLOT.value:
        run_plot.main(args[1:])
        return
    if args[0] == ScriptType.INSTALL_DEPS.value:
        run_install_deps.main(args[1:])
        return
    if args[0] == ScriptType.TESTS.value:
        run_tests.main(args[1:])
        return
    if args[0] == ScriptType.MEASUREMENTS.value:
        run_measurements.main(args[1:])
        return
    if args[0] == ScriptType.MONITORING.value:
        run_monitoring.main(args[1:])
        return
    if args[0] == ScriptType.BUILD.value:
        run_build.main(args[1:])
        return
    if args[0] == ScriptType.DEPLOY.value:
        run_deploy.main(args[1:])
        return
    if args[0] == ScriptType.HELP.value:
        print_help_message()
        return
    print_error_message()


# Main script for run command. Called from run.sh.
if __name__ == '__main__':
    # Register handler in case of user interruption.
    signal.signal(signal.SIGINT, cancel_execution_handler)

    # Parse arguments and run_{command}.py script.
    parse_input_arguments(sys.argv[1:])
