#!/bin/bash


DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$2" ]; then
    echo "Syntax: ./start-vm.sh <clone id> <vm ip>"
    echo "Clone id should an integer higher than zero which is not being used by another clone vm."
    echo "Please use a free IP 172.18.[0,255].[3,255]. The IP will be used to create a firecracker directory and to route requests to the vm."
    exit 0
fi

# ID of the VM clone. Used to prepare internal ips.
CLONE_ID=$1

# VM ip accessible to the outside (unique).
PUBLIC_VM_IP=$2

# ID of the vm (based on the public ip).
VM_ID=$(echo $PUBLIC_VM_IP | tr . -)

CHROOT_DIR=$DIR/$PUBLIC_VM_IP
mkdir -p $CHROOT_DIR/firecracker-v1.1.0-x86_64/$VM_ID/root/
touch    $CHROOT_DIR/firecracker-v1.1.0-x86_64/$VM_ID/root/firecracker.log

# create namespace
sudo ip netns add ns$CLONE_ID

sudo "$DIR"/release-v1.1.0-x86_64/jailer-v1.1.0-x86_64 \
       --id $VM_ID \
       --exec-file "$DIR"/release-v1.1.0-x86_64/firecracker-v1.1.0-x86_64 \
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
