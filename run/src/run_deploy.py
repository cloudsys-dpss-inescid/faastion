#!/usr/bin/python3
import os.path
import sys
import threading
import time

from run_globals import LAMBDA_MANAGER_DIR, CLUSTER_MANAGER_DIR, LOAD_BALANCER_DIR, DEPLOY_SCRIPT, \
    MAX_MANAGER_WAKEUP_TIME
from run_utils import print_message, MessageType


# Core methods.
def run_load_balancer():
    print_message("Running load balancer...", MessageType.INFO)
    os.system("bash {load_balancer_deploy_script}".format(
        load_balancer_deploy_script=os.path.join(LOAD_BALANCER_DIR, DEPLOY_SCRIPT)))
    print_message("Running load balancer...done", MessageType.INFO)


def run_cluster_manager():
    print_message("Running cluster manager...", MessageType.INFO)
    os.system("bash {cluster_manager_deploy_script}".format(
        cluster_manager_deploy_script=os.path.join(CLUSTER_MANAGER_DIR, DEPLOY_SCRIPT)))
    print_message("Running cluster manager...done", MessageType.INFO)


def run_lambda_manager():
    print_message("Running lambda manager...", MessageType.INFO)
    os.system("bash {lambda_manager_deploy_script}".format(
        lambda_manager_deploy_script=os.path.join(LAMBDA_MANAGER_DIR, DEPLOY_SCRIPT)))
    print_message("Running lambda manager...done", MessageType.INFO)


def do(filter_list):
    filter_list_empty = len(filter_list) == 0
    filter_set = set(filter_list.split(","))

    if "lb" in filter_set or filter_list_empty:
        threading.Thread(target=run_load_balancer, args=()).start()
        print_message("Waiting for a load balancer to become available...", MessageType.INFO)
        time.sleep(MAX_MANAGER_WAKEUP_TIME)
    if "cm" in filter_set or filter_list_empty:
        threading.Thread(target=run_cluster_manager, args=()).start()
        print_message("Waiting for a cluster manager to become available...", MessageType.INFO)
        time.sleep(MAX_MANAGER_WAKEUP_TIME)
    if "lm" in filter_set or filter_list_empty:
        print_message("Waiting for a lambda manager to become available...", MessageType.INFO)
        threading.Thread(target=run_lambda_manager, args=()).start()
        time.sleep(MAX_MANAGER_WAKEUP_TIME)


# Main function.
def main(args):
    filter_list = ""
    if len(args) == 0:
        # Run all.
        pass
    elif len(args) == 1:
        # Run only selected components.
        filter_list = args[0]
    else:
        print_message("Too much arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)

    do(filter_list)


if __name__ == '__main__':
    main(sys.argv[1:])
