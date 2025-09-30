#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

DISK=$DIR/disk

hsdisk=$1

if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters."
    echo "Syntax: build_vm_image.sh <output vm disk path>"
    exit 1
fi

# Build out init program.
gcc -c $DIR/init.c -o $DIR/init.o
gcc -o $DIR/init $DIR/init.o

# Prepare directory used to setup the filesystem.
rm -rf $DISK &> /dev/null
mkdir -p $DISK

# If we don't have a base filesystem, create one.
if [ ! -f $DIR/base.ext4 ];
then
    # Create an empty 2gb partition.
    dd if=/dev/zero of=$DIR/base.ext4 bs=1M count=4096
    mkfs.ext4 $DIR/base.ext4
    # Mount it, add permissions
    sudo mount $DIR/base.ext4 $DISK
    sudo chown -R $(id -u -n):$(id -g -n) $DISK
    # Use an argo-hotspot docker to copy the entire image to the mounted dir.
    docker export $(docker create argo-hotspot:latest) | tar -xC $DISK
    # Revert permissions and unmount.
    sudo chown -R root:root $DISK
    sudo umount $DISK
fi

# Copy the base filesystem into a new one, mount, and setup permissions.
cp $DIR/base.ext4 $hsdisk
sudo mount $hsdisk $DISK
sudo chown -R $(id -u -n):$(id -g -n) $DISK

# Copy init.
cp $DIR/init $DISK

# Unmount image and remove directory used for the mount.
sudo chown -R root:root $DISK
sudo umount $DISK
rm -rf $DISK
