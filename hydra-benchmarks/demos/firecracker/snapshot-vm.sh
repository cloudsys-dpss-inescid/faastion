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

curl --unix-socket $VM_SOCKET -i \
    -X PATCH "http://localhost/vm" \
    -d "{ \"state\": \"Paused\" }"

curl --unix-socket $VM_SOCKET -i \
    -X PUT "http://localhost/snapshot/create" \
    -d "{
        \"snapshot_type\": \"Full\",
        \"snapshot_path\": \"$VM_SNAP_FILE\",
        \"mem_file_path\": \"$VM_SNAP_MEM\"
    }"

curl --unix-socket $VM_SOCKET -i \
    -X PATCH "http://localhost/vm" \
    -d "{ \"state\": \"Resumed\" }"
