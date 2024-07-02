#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$1" ]; then
    echo "Please use a free IP 172.18.[0,255].[2,255]. The IP will be used to create a firecracker directory and tap."
    exit 0
fi

CHROOT_DIR=$DIR/$1
VM_ID=$(echo $1 | tr . -)
mkdir -p $CHROOT_DIR

sudo "$DIR"/release-v1.1.0-x86_64/jailer-v1.1.0-x86_64 \
       --id $VM_ID \
       --exec-file "$DIR"/release-v1.1.0-x86_64/firecracker-v1.1.0-x86_64 \
       --uid $(id -u $USER) \
       --gid $(id -g $USER) \
       --chroot-base-dir $CHROOT_DIR \
       -- --api-sock firecracker.socket
