#!/usr/bin/python3
import glob
import os
import shlex
import subprocess
import sys

from run_utils import write_file, remove_file, write_json_file, print_message, MessageType, read_json_file
from run_globals import MAX_VERBOSITY_LVL, MANAGER_CONFIG_PATH, SETUP_DB_VM_LOC, SETUP_DB_VM_FILE

# Global variables.
VERBOSITY_LVL = 0

SETUP_DB_VM_TMP = "setup_debian_vm_tmp.sh"
SETUP_SCRIPT = '''#!/usr/bin/bash
cd {location}
bash {file}
'''

MASK = "24"
GET_DEF_GATEWAY_FILE = "get_default_gateway.sh"
GET_DEF_GATEWAY_SCRIPT = '''
#!/usr/bin/bash
ip a | grep `ip r | grep default | head -n 1 | awk '{print $5}'` | grep inet | awk '{print $2}'
'''


def set_verbosity(flag):
    global VERBOSITY_LVL
    v_count = flag.count("v")
    if v_count != len(flag):
        print_message("Verbosity flag should be v or vv instead of {flag}. Output verbosity level will fallback to 0."
                      .format(flag=flag), MessageType.WARN)
        return
    VERBOSITY_LVL = min(v_count, MAX_VERBOSITY_LVL)
    print_message("Output verbosity level is set to {level}.".format(level=VERBOSITY_LVL), MessageType.SPEC)


# Core methods.
def run(command):
    outs, errs = subprocess.Popen(shlex.split(command),
                                  stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
    outs, errs = outs.decode(sys.stdout.encoding), errs.decode(sys.stdout.encoding)
    if len(outs) > 0 and VERBOSITY_LVL > 1:
        print_message(outs, MessageType.NO_HEADER)
    if len(errs) > 0 and VERBOSITY_LVL > 0:
        print_message("Command ({}) error log:\n{}".format(command, errs), MessageType.WARN)
    return outs


def make_kernel_writable():
    print_message("Making kernel writable...", MessageType.INFO)
    kernel_location = os.path.join("/", "boot")
    kernel_files = sorted(glob.glob(os.path.join(kernel_location, "vmlinuz-*")))
    if len(kernel_files) == 0:
        print_message("Something went wrong! No kernel files found!", MessageType.ERROR)
        exit(1)
    run("sudo chmod +w {}".format(os.path.join(kernel_location, kernel_files[-1])))
    print_message("Making kernel writable...done", MessageType.INFO)


def install_world():
    print_message("Installing world...", MessageType.INFO)
    run("sudo apt-get update")
    # Installing GCC and system utils...
    run("sudo apt-get install -y gcc libz-dev libguestfs-tools bridge-utils")
    # Installing qemu...
    run("sudo apt-get install -y qemu qemu-kvm")
    # Installing tools for testing and plotting...
    run("sudo apt-get install -y apache2-utils gnuplot")
    print_message("Installing the world...done", MessageType.INFO)


def download_resources():
    print_message("Downloading resources...", MessageType.INFO)
    print_message("Downloading resources...done", MessageType.INFO)


def setup_debian_vm():
    print_message("Setup debian vm...", MessageType.INFO)
    write_file(SETUP_DB_VM_TMP, SETUP_SCRIPT.format(location=SETUP_DB_VM_LOC, file=SETUP_DB_VM_FILE))
    os.system("bash {script}".format(script=SETUP_DB_VM_TMP))
    remove_file(SETUP_DB_VM_TMP)
    print_message("Setup debian vm...done", MessageType.INFO)


def setup_bridge():
    print_message("Setup bridge...", MessageType.INFO)

    # Get default gateway.
    write_file(GET_DEF_GATEWAY_FILE, GET_DEF_GATEWAY_SCRIPT)
    default_gateway = run("bash {file}".format(file=GET_DEF_GATEWAY_FILE))
    remove_file(GET_DEF_GATEWAY_FILE)

    # Add gateway address in configs/manager/default-lambda-manager.json.
    manager_config_file = read_json_file(MANAGER_CONFIG_PATH)
    manager_config_file['gateway'] = "{}/{}".format(default_gateway[:-1], MASK)
    write_json_file(MANAGER_CONFIG_PATH, manager_config_file)

    print_message("Setup bridge...done", MessageType.INFO)


# Main function.
def main(args):
    if len(args) == 0:
        print_message("Output verbosity level will be 0.", MessageType.SPEC)
    elif len(args) == 1:
        set_verbosity(args[0])
    else:
        print_message("Too much arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)
    make_kernel_writable()
    install_world()
    download_resources()
    setup_debian_vm()
    setup_bridge()


if __name__ == '__main__':
    main(sys.argv[1:])
