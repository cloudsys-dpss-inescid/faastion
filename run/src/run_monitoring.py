#!/usr/bin/python3
import os
import sys

from run_utils import print_message, MessageType
from run_globals import WEB_UI_DIR


def do():
    print_message("Starting Grafana...", MessageType.INFO)
    os.system("sudo docker start grafana")
    print_message("Starting Grafana...done", MessageType.INFO)

    print_message("Starting InfluxDB...", MessageType.INFO)
    os.system("sudo docker start influxdb2")
    print_message("Starting InfluxDB...done", MessageType.INFO)

    print_message("Starting WebUI...", MessageType.INFO)
    os.system("cd {webui_dir} && ng serve --port 30001 --host 0.0.0.0 &".format(webui_dir=WEB_UI_DIR))
    print_message("Starting WebUI...done", MessageType.INFO)


# Main function.
def main(args):
    if len(args) > 0:
        print_message("Too much arguments - {}!".format(len(args)), MessageType.ERROR)
        exit(1)
    do()


if __name__ == '__main__':
    main(sys.argv[1:])
