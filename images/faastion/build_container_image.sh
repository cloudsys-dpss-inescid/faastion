#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor
LIB_DIR=$GRAALVISOR_HOME/shared/
GRAALVISOR_BINARY=$GRAALVISOR_HOME/build/native-image/polyglot-proxy

GREEN='\033[0;32m'
NC='\033[0m' # No Color

if [ -z "$ARGO_HOME" ]; then
    echo "Please set ARGO_HOME first."
    exit 1
fi

# Prepare directory used to setup the filesystem.
DISK=$DIR/disk
rm -rf $DISK &> /dev/null
mkdir -p $DISK/
# mkdir -p $DISK/graalvisor/build/native-image $DISK/$LIB_DIR

echo '#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

if [ "$1" = "--enable-lpi" ]; then
    export LPI=true
elif [ "$1" = "--enable-early-booking" ]; then
    export faastlane=true
fi

library_path="'$BUILD_HOME':'$LIBC_HOME'/lib:/usr/local/lib:/lib/x86_64-linux-gnu"
env GLIBC_TUNABLES="glibc.rtld.nns=16" LD_LIBRARY_PATH="$library_path" LD_PRELOAD="libmem.so" $DIR/build/native-image/polyglot-proxy
' > $DISK/start.sh

cd $ARGO_HOME
git archive -o faastion.zip --prefix="faastion/" HEAD
cd - &> /dev/null

# Copy graalvisor and init.
cp $DIR/glibc-2.35.patch $DISK/.
cp $DIR/glibc-2.35.tar.gz $DISK/.
mv $ARGO_HOME/faastion.zip $DISK/.
# cp $BUILD_HOME/*.so $DISK/$BUILD_HOME/.
# cp -r $LIBC_HOME/* $DISK/$LIBC_HOME/.
# cp $GRAALVISOR_BINARY $DISK/graalvisor/build/native-image/polyglot-proxy

# Build docker.
docker build -t faastion $DIR

echo -e "${GREEN}Copying library to host...${NC}"
docker run -d --rm --network host --name sbox -it --entrypoint sleep faastion infinity &> /dev/null
docker cp sbox:/faastion/graalvisor/shared/ $DIR/../../graalvisor/
docker container stop sbox &> /dev/null
echo -e "${GREEN}Copying library to host... done!${NC}"

# Remove directory used to create the image.
rm -rf $DISK &> /dev/null
