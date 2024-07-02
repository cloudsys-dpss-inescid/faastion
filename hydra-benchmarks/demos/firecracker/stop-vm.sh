#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$1" ]; then
    echo "Please use a free IP 172.18.[0,255].[2,255]. The IP will be used to create a firecracker directory and tap."
    exit 0
fi

VM_IP=$1
VM_DIR=$DIR/$VM_IP
VM_SOCKET=$VM_DIR/firecracker.socket
HOST_TAP=tap$VM_IP
HOST_DEV=$(ip route | grep default | awk '{print $5}')

# Kill the process that has that file open.
kill $(fuser $VM_SOCKET  2>&1 | awk '{print $2}')

# Remove the file.
rm $VM_SOCKET

# Remove vm tap.
sudo ip link delete $HOST_TAP
