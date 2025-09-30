#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

function build_vm_image {
    IMAGE=$1
    cd "$DIR"/"$IMAGE"
    bash build_vm_image.sh "$IMAGE".img
    rm "$DIR"/"$IMAGE"/base.ext4 "$DIR"/"$IMAGE"/init "$DIR"/"$IMAGE"/init.o "$DIR"/"$IMAGE"/random.o &> /dev/null
    cd "$DIR"
}

function build_container_image {
    IMAGE=$1
    cd "$DIR"/"$IMAGE"
    bash build_container_image.sh
    cd "$DIR"
}

read -p "Graalvisor container (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_container_image graalvisor
fi

read -p "Graalvisor VM (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_container_image graalvisor
    build_vm_image graalvisor
fi

read -p "HotSpot container (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_container_image hotspot
    build_container_image hotspot-agent
fi

read -p "HotSpot VM (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_vm_image hotspot
    build_vm_image hotspot-agent
fi

read -p "Java OpenWhisk VM (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_vm_image java-openwhisk
fi

read -p "Python OpenWhisk VM (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_vm_image python-openwhisk
fi

read -p "JavaScript OpenWhisk VM (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_vm_image javascript-openwhisk
fi
