#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

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

echo '#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"

if [ "$1" = "--enable-lpi" ]; then
    export LPI=true
    export pku_isolation=on
    ld_preload="LD_PRELOAD=libmem.so"
elif [ "$1" = "--mpk-only" ]; then
    export faastlane=true
    export pku_isolation=on
    ld_preload="LD_PRELOAD=libmem.so"
fi

library_path=/faastion/core/shared:/glibc-2.35/build/install/lib:/lib/x86_64-linux-gnu:/usr/local/lib
env GLIBC_TUNABLES="glibc.rtld.nns=16" LD_LIBRARY_PATH="$library_path" $ld_preload ./faastion/core/build/native-image/polyglot-proxy
' > $DISK/start.sh

# Create faastion zip file
cd $ARGO_HOME
git archive -o faastion.zip --prefix="faastion/" HEAD
cd - &> /dev/null

# Copy faastion repo and glibc
cp $DIR/glibc-2.35.patch $DISK/.
cp $DIR/glibc-2.35.tar.gz $DISK/.
mv $ARGO_HOME/faastion.zip $DISK/.

# Build docker.
docker build -t faastion $DIR

echo -e "${GREEN}Copying library to host...${NC}"
docker run -d --rm --network host --name sbox -it --entrypoint sleep faastion infinity &> /dev/null
docker cp sbox:/faastion/core/shared/ $DIR/../../core/
docker container stop sbox &> /dev/null
echo -e "${GREEN}Copying library to host... done!${NC}"

# Remove directory used to create the image.
rm -rf $DISK &> /dev/null
