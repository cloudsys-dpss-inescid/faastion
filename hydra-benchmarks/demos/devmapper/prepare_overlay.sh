#!/bin/bash

# Inpired by
# [1] https://jvns.ca/blog/2021/01/27/day-47--using-device-mapper-to-manage-firecracker-images/
# [2] https://blog.oddbit.com/post/2018-01-25-fun-with-devicemapper-snapshot/

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

BASENAME=$1
BASEIMAGE=$2
OVERLAYNAME=$3
OVERLAY=$4
SZ=`sudo blockdev --getsz $BASEIMAGE`

# Step 1: Create an empty image with the size of BASEIMAGE.
qemu-img create -f raw $OVERLAY $(stat -c%s $BASEIMAGE)

# Step 2: Create another loop device for the OVERLAY file.
LOOP2=$(sudo losetup --find --show $OVERLAY)
echo "$LOOP2" > $DIR/$OVERLAYNAME.loop 

# Step 3: Create the final device mapper for the overlay image.
echo "0 $SZ snapshot /dev/mapper/$BASENAME $LOOP2 P 8" | sudo dmsetup create $OVERLAYNAME
