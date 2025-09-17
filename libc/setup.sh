#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)

# Download and extract glibc
cd $DIR
wget https://ftp.gnu.org/gnu/libc/glibc-2.35.tar.gz
tar xzvf glibc-2.35.tar.gz
cd - &> /dev/null

# Apply libc patch
cd $DIR/glibc-2.35
patch -s -p1 < ../glibc-2.35.patch
mkdir build
cd - &> /dev/null

# Build libc
cd $DIR/glibc-2.35/build
../configure --prefix=$(pwd)/install
make -j20
make install -j20