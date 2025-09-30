#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

source $DIR/build_shared.sh

DISK=$DIR/disk

GRAALVISOR_BINARY=$DIR/../../graalvisor/build/native-image/polyglot-proxy
gvdisk=$1

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
    # Create an empty 4gb partition.
    dd if=/dev/zero of=$DIR/base.ext4 bs=1M count=4096
    mkfs.ext4 $DIR/base.ext4
    # Mount it, add permissions
    sudo mount $DIR/base.ext4 $DISK
    sudo chown -R $(id -u -n):$(id -g -n) $DISK
    # Use a debian docker to copy the entire image to the mounted dir.
    docker export $(docker create graalvisor) | tar -xC $DISK
    # Revert permissions and unmount.
    sudo chown -R root:root $DISK
    sudo umount $DISK
fi

# Copy the base filesystem into a new one, mount, and setup permissions.
cp $DIR/base.ext4 $gvdisk
sudo mount $gvdisk $DISK
sudo chown -R $(id -u -n):$(id -g -n) $DISK

# Copy graalvisor and init.
cp $DIR/init $DISK

# Unmount image and remove directory used for the mount.
sudo chown -R root:root $DISK
sudo umount $DISK
rm -r $DISK
