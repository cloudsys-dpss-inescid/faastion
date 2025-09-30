#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

export app_dir=$(pwd)/apps
mkdir -p $app_dir
bash $(DIR)/../../../graalvisor/graalvisor-gdb
