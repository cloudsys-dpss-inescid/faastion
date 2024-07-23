#!/bin/bash

MUSL=$ARGO_HOME/resources/x86_64-linux-musl-native/x86_64-linux-musl
JAIL=/tmp/musl-chroot

# Function is important to ensure that we umount proc.
function cleanup {
    sudo umount $JAIL/proc
    sudo umount -l $JAIL/dev
}
trap cleanup EXIT

# Create jail for chroot.
sudo rm -rf $JAIL
cp -r $MUSL $JAIL

# Add proc to it.
mkdir $JAIL/proc
mkdir $JAIL/dev
sudo mount --bind -o ro /proc $JAIL/proc
sudo mount --bind -o ro /dev $JAIL/dev

# Add test binary to jail.
cp -r ./test             $JAIL/ # TODO - test path should not be relative.
cp -r *.so $JAIL/ # TODO - test path should not be relative.

# Compile our graalvisor chroot launcher and run it.
gcc -o launch-in-chroot launch-in-chroot.c
sudo setarch -R ./launch-in-chroot $JAIL "LD_LIBRARY_PATH=" "LD_PRELOAD=/libc_callgate.so" /test
#sudo setarch -R gdb --args ./launch-in-chroot $JAIL "LD_LIBRARY_PATH=" "LD_PRELOAD=/libc_callgate.so" /test
