#!/bin/bash


DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$3" ]; then
    echo "Syntax: ./start-vm.sh <clone id> <vm ip> <vm image name>"
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

CHROOT_DIR=$DIR/$IMAGE_NAME/$PUBLIC_VM_IP
mkdir -p $CHROOT_DIR/firecracker/$VM_ID/root/
touch    $CHROOT_DIR/firecracker/$VM_ID/root/firecracker.log

# create namespace
sudo ip netns add ns$CLONE_ID

sudo jailer \
       --id $VM_ID \
       --exec-file $(which firecracker) \
       --uid 0 \
       --gid 0 \
       --netns /var/run/netns/ns$CLONE_ID \
       --chroot-base-dir $CHROOT_DIR \
       -- \
       --api-sock firecracker.socket \
       --log-path firecracker.log \
       --level Debug \
       --show-level \
       --show-log-origin
