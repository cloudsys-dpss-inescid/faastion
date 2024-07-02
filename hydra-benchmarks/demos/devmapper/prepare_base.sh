#!/bin/bash

# Inpired by
# [1] https://jvns.ca/blog/2021/01/27/day-47--using-device-mapper-to-manage-firecracker-images/
# [2] https://blog.oddbit.com/post/2018-01-25-fun-with-devicemapper-snapshot/

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

BASENAME=$1
BASEIMAGE=$2
SZ=`sudo blockdev --getsz $BASEIMAGE`

# Step 1: Create a loop device for the BASEIMAGE file (like /dev/loop16)
LOOP=$(sudo losetup --find --show --read-only $BASEIMAGE)
echo "$LOOP" > $DIR/$BASENAME.loop 
# Step 2: Create device mapper for the base image.
printf "0 $SZ linear $LOOP 0\n$SZ $SZ zero" | sudo dmsetup create $BASENAME
