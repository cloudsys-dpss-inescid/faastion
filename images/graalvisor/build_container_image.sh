#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

# source $DIR/build_shared.sh

DISK=$DIR/disk

GRAALVISOR_HOME=$DIR/../../graalvisor
GRAALVISOR_BINARY=$GRAALVISOR_HOME/build/native-image/polyglot-proxy
GRAALVISOR_SCRIPT=$GRAALVISOR_HOME/graalvisor
GRAALVISOR_ALLOCATOR=$GRAALVISOR_HOME/src/main/c/svm-snapshot/deps/dlmalloc/hydralloc.so

# Prepare directory used to setup the filesystem.
rm -rf $DISK &> /dev/null
mkdir -p $DISK
mkdir -p $DISK/build/native-image
mkdir -p $DISK/src/main/c/svm-snapshot/deps/dlmalloc

# Copy files necessary to execute non-java functions.
#copy_polyglot_deps

# Copy graalvisor and init.
cp $GRAALVISOR_BINARY $DISK/build/native-image/polyglot-proxy
cp $GRAALVISOR_SCRIPT $DISK/graalvisor
cp $GRAALVISOR_ALLOCATOR $DISK/src/main/c/svm-snapshot/deps/dlmalloc/hydralloc.so

# Build docker.
docker build --tag=hydra $DIR

# Remove directory used to create the image.
rm -r $DISK
