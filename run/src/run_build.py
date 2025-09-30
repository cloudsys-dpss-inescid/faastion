#!/usr/bin/python3
import os.path
import sys

from run_globals import LAMBDA_MANAGER_DIR, PROXY_DIR, GRAALVISOR_LIB_DIR, BUILD_SCRIPT, CLUSTER_MANAGER_DIR, LOAD_BALANCER_DIR
from run_utils import print_message, MessageType


# Core methods.
def build_load_balancer():
    print_message("Building load balancer...", MessageType.INFO)
    os.system("bash {load_balancer_build_script}".format(
        load_balancer_build_script=os.path.join(LOAD_BALANCER_DIR, BUILD_SCRIPT)))
    print_message("Building load balancer...done", MessageType.INFO)


def build_lambda_proxy():
    print_message("Building lambda proxy...", MessageType.INFO)
    os.system("bash {lambda_proxy_build_script}".format(
        lambda_proxy_build_script=os.path.join(PROXY_DIR, BUILD_SCRIPT)))
    print_message("Building lambda proxy...done", MessageType.INFO)


def build_cluster_manager():
    print_message("Building cluster manager...", MessageType.INFO)
    os.system("bash {cluster_manager_build_script}".format(
        cluster_manager_build_script=os.path.join(CLUSTER_MANAGER_DIR, BUILD_SCRIPT)))
    print_message("Building cluster manager...done", MessageType.INFO)


def build_lambda_manager():
    print_message("Building lambda manager...", MessageType.INFO)
    os.system("bash {lambda_manager_build_script}".format(
        lambda_manager_build_script=os.path.join(LAMBDA_MANAGER_DIR, BUILD_SCRIPT)))
    print_message("Building lambda manager...done", MessageType.INFO)


def build_graalvisor_library():
    print_message("Building graalvisor library...", MessageType.INFO)
    os.system("bash {graalvisor_library_build_script}".format(
        graalvisor_library_build_script=os.path.join(GRAALVISOR_LIB_DIR, BUILD_SCRIPT)))
    print_message("Building graalvisor library...done", MessageType.INFO)


def do(filter_list):
    filter_list_empty = len(filter_list) == 0
    filter_set = set(filter_list.split(","))
    if "lb" in filter_set or filter_list_empty:
        build_load_balancer()
    if "lp" in filter_set or filter_list_empty:
        build_lambda_proxy()
    if "cm" in filter_set or filter_list_empty:
        build_cluster_manager()
    if "lm" in filter_set or filter_list_empty:
        build_lambda_manager()
    if "gv-lib" in filter_set or filter_list_empty:
        build_graalvisor_library()


# Main function.
def main(args):
    filter_list = ""
    if len(args) == 0:
        # Build all.
        pass
    elif len(args) == 1:
        # Build only selected components.
        filter_list = args[0]
    else:
        print_message("Too much arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)

    do(filter_list)


if __name__ == '__main__':
    main(sys.argv[1:])
