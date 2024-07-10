#!/bin/bash

MUSL=$ARGO_HOME/resources/x86_64-linux-musl-native/x86_64-linux-musl
JAIL=/tmp/musl-chroot

# Function is important to ensure that we umount proc.
function cleanup {
    sudo umount $JAIL/proc
    sudo umount -l $JAIL/dev
}
trap cleanup EXIT


# Preparing faastion directory.

# Create jail for chroot.
sudo rm -rf $JAIL
cp -r $MUSL $JAIL

# Add proc to it.
mkdir $JAIL/proc
mkdir $JAIL/dev
sudo mount --bind -o ro /proc $JAIL/proc
sudo mount --bind -o ro /dev $JAIL/dev

# Add graalvisor and faastion directories.
mkdir -p $JAIL/tmp/faastion
mkdir -p $JAIL/tmp/fifo
mkdir -p $JAIL/tmp/ret
mkdir -p $JAIL/$ARGO_HOME/graalvisor
cp -r $ARGO_HOME/graalvisor/build $JAIL/$ARGO_HOME/graalvisor/

# Compile our graalvisor chroot launcher and run it.
gcc -o graalvisor-chroot graalvisor-chroot.c
sudo ./graalvisor-chroot \
    $JAIL \
    "LD_LIBRARY_PATH=$ARGO_HOME/graalvisor/build/libs" \
    "LD_PRELOAD=$ARGO_HOME/graalvisor/build/libs/libpreload.so" \
    $ARGO_HOME/graalvisor/build/native-image/polyglot-proxy
