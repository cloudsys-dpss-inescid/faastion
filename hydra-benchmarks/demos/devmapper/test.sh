#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

BASE_ID=baseimage
BASE_PATH=$DIR/../firecracker/hello-rootfs.ext4
OVER_ID=overlay
OVER_PATH=$DIR/overlay.ext4

# Prepare dir where we will test the image.
mkdir $DIR/rootfs

echo "Preparing base image."
$DIR/prepare_base.sh $BASE_ID $BASE_PATH
sleep 1

echo "Preparing overlay image."
time $DIR/prepare_overlay.sh $BASE_ID $BASE_PATH $OVER_ID $OVER_PATH
sleep 1

echo "Testing overlay image."
sudo mount -t ext4 /dev/mapper/overlay $DIR/rootfs
ls $DIR/rootfs
sudo touch $DIR/rootfs/ola
ls $DIR/rootfs
sudo umount $DIR/rootfs

echo "Deleting overlay image."
$DIR/delete_overlay.sh $OVER_ID $OVER_PATH
sleep 1

echo "Deleting base image."
$DIR/delete_base.sh $BASE_ID

# Delete our test dir.
rm -r $DIR/rootfs
