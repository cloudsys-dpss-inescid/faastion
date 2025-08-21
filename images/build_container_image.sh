#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor
BUILD_HOME=$GRAALVISOR_HOME/build/libs/
GRAALVISOR_BINARY=$GRAALVISOR_HOME/build/native-image/polyglot-proxy

# Prepare directory used to setup the filesystem.
DISK=$DIR/disk
rm -rf $DISK &> /dev/null
mkdir -p $DISK/graalvisor/build/native-image $DISK/$LIBC_HOME $DISK/$BUILD_HOME

echo '#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

library_path="'$BUILD_HOME':'$LIBC_HOME'/lib:/lib/x86_64-linux-gnu"
env GLIBC_TUNABLES="glibc.rtld.nns=16" LD_LIBRARY_PATH="$library_path" LD_PRELOAD="libmem.so" $DIR/build/native-image/polyglot-proxy
' > $DISK/graalvisor/start.sh

# Copy graalvisor and init.
cp $BUILD_HOME/*.so $DISK/$BUILD_HOME/.
cp $BUILD_HOME/*-proc $DISK/$BUILD_HOME/. # required by faastion LPI
cp -r $LIBC_HOME/* $DISK/$LIBC_HOME/.
cp /usr/lib/libigraph.so.3 $DISK/$BUILD_HOME/. # required by search algorithms
cp $GRAALVISOR_BINARY $DISK/graalvisor/build/native-image/polyglot-proxy

# Build docker.
docker build --build-arg build_home=$BUILD_HOME --build-arg libc_home=$LIBC_HOME -t graalvisor $DIR

# Remove directory used to create the image.
rm -rf $DISK &> /dev/null
