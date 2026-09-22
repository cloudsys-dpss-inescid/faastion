#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

function build_vm_image {
    IMAGE=$1
    cd "$DIR"/"$IMAGE"
    bash build_vm_image.sh "$IMAGE".img
    rm "$DIR"/"$IMAGE"/base.ext4 "$DIR"/"$IMAGE"/init "$DIR"/"$IMAGE"/init.o "$DIR"/"$IMAGE"/random.o &> /dev/null
    cd "$DIR"
}

read -p "Java OpenWhisk VM (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    build_vm_image java-openwhisk
fi
