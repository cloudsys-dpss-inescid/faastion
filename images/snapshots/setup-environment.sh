#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

cd $DIR &> /dev/null

if [ -z "$ARGO_HOME" ]; then
    echo "Please set your ARGO_HOME environment variable."
    exit 0
fi

echo "1" | sudo tee /proc/sys/net/ipv4/ip_forward

echo "Checking if you have a linux kernel image..."
if [ ! -f $DIR/hello-vmlinux.bin ];
then
    cp "$ARGO_HOME"/resources/hello-vmlinux.bin $DIR/hello-vmlinux.bin
fi
echo "Checking if you have a linux kernel image... done!"

echo "Checking if you have a base rootfs..."
for image_name in graalvisor hotspot hotspot-agent java-openwhisk; do
    if [ ! -f $DIR/$image_name/$image_name.img ];
    then
        # Ensure we have directory created.
        mkdir $DIR/$image_name &> /dev/null
        cp "$ARGO_HOME"/images/$image_name/$image_name.img $DIR/$image_name/$image_name.img
    fi
done
echo "Checking if you have a base rootfs... done!"

cd - &> /dev/null
