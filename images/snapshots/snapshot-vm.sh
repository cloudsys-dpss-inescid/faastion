#!/bin/bash


DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$3" ]; then
    echo "Syntax: ./snapshot-vm.sh <clone id> <vm ip> <vm image name>"
    echo "Clone id should an integer higher than zero which is not being used by another clone vm."
    echo "Please use a free IP 172.18.[0,255].[3,255]. The IP will be used to create a firecracker directory and to route requests to the vm."
    exit 0
fi

# ID of the VM clone. Used to prepare internal ips.
CLONE_ID=$1

# VM ip accessible to the outside (unique).
PUBLIC_VM_IP=$2

# Image to be used (graalvisor, hotspot, hotspot-agent).
IMAGE_NAME=$3

# ID of the vm (based on the public ip).
VM_ID=$(echo $PUBLIC_VM_IP | tr . -)

# TODO: this is a dirty hack to avoid having socket path too long.
if [ ! -L /tmp/snapshots ]; then
    ln -s $DIR /tmp/snapshots
fi
# Socket that will be used to control the vm.
VM_SOCKET=/tmp/snapshots/$IMAGE_NAME/$PUBLIC_VM_IP/root/firecracker.socket

# Snapshot file paths (files insire the chroot).
VM_SNAP_FILE=snapshot_file
VM_SNAP_MEM=mem_file

echo $VM_SNAP_FILE
echo $VM_SNAP_MEM

sudo curl --unix-socket $VM_SOCKET -i \
    -X PATCH "http://localhost/vm" \
    -d "{ \"state\": \"Paused\" }"

sudo curl --unix-socket $VM_SOCKET -i \
    -X PUT "http://localhost/snapshot/create" \
    -d "{
        \"snapshot_type\": \"Full\",
        \"snapshot_path\": \"$VM_SNAP_FILE\",
        \"mem_file_path\": \"$VM_SNAP_MEM\"
    }"

sudo curl --unix-socket $VM_SOCKET -i \
    -X PATCH "http://localhost/vm" \
    -d "{ \"state\": \"Resumed\" }"
