#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$1" ]; then
    echo "Please use a free IP 172.18.[0,255].[2,255]. The IP will be used to create a firecracker directory and tap."
    exit 0
fi

VM_IP=$1
VM_ID=$(echo $VM_IP | tr . -)
HOST_TAP=tap$VM_IP

# Kill the process that has that file open.
kill $(ps aux | grep $VM_ID | grep jailer | awk '{print $2}')

# Remove vm tap.
sudo ip link delete $HOST_TAP
