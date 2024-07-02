#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$1" ]; then
    echo "Please use a free IP 172.18.[0,255].[2,255]. The IP will be used to create a firecracker directory and tap."
    exit 0
fi

VM_IP=$1
VM_DIR=$DIR/$VM_IP
VM_SOCKET=$VM_DIR/firecracker.socket
VM_SNAP_FILE=$VM_DIR/snapshot_file
VM_SNAP_MEM=$VM_DIR/mem_file
VM_TAP=tap$VM_IP

# We need to recreate the network setup (see config-vm.sh).
sudo ip tuntap add dev "$VM_TAP" mode tap
sudo brctl addif docker0 $VM_TAP
sudo ip link set dev "$VM_TAP" up

curl --unix-socket $VM_SOCKET -i \
    -X PUT "http://localhost/snapshot/load" \
    -d "{
        \"snapshot_path\": \"$VM_SNAP_FILE\",
        \"mem_file_path\": \"$VM_SNAP_MEM\",
        \"enable_diff_snapshots\": false,
        \"resume_vm\": true
    }"
