#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor
BUILD_HOME=$GRAALVISOR_HOME/build/libs/
GRAALVISOR_BINARY=$GRAALVISOR_HOME/build/native-image/polyglot-proxy

if [ -z "$ARGO_HOME" ]; then
    echo "Please set ARGO_HOME first."
    exit 1
fi

# Prepare directory used to setup the filesystem.
DISK=$DIR/disk
rm -rf $DISK &> /dev/null
mkdir -p $DISK/graalvisor/build/native-image $DISK/$BUILD_HOME

echo '#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

if [ "$1" = "--enable-lpi" ]; then
    export LPI=true
elif [ "$1" = "--enable-early-booking" ]; then
    export faastlane=true
fi

library_path="'$BUILD_HOME':/glibc-2.35/build/install/lib:/usr/local/lib:/lib/x86_64-linux-gnu"
env GLIBC_TUNABLES="glibc.rtld.nns=16" LD_LIBRARY_PATH="$library_path" LD_PRELOAD="libmem.so" $DIR/build/native-image/polyglot-proxy
' > $DISK/graalvisor/start.sh

# Copy graalvisor and init.
cp $BUILD_HOME/*.so $DISK/$BUILD_HOME/.
cp $DIR/glibc-2.35.patch $DISK/.
cp $GRAALVISOR_BINARY $DISK/graalvisor/build/native-image/polyglot-proxy

# Build docker.
docker build --build-arg libc_home=$LIBC_HOME -t faastion $DIR

# Remove directory used to create the image.
rm -rf $DISK &> /dev/null
