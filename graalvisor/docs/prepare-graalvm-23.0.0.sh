#!/bin/bash

GRAALVM=graalvm-community-openjdk-17.0.7+7.1

if [ "$#" -ne 2  ]; then
    echo "Illegal number of parameters."
    echo "This script is meant to be called like this:"
    echo "> docker run -it -v \$PWD:/\$PWD --network host -w \$PWD --rm debian bash \$PWD/prepare-graalvm-23.0.0.sh \$(id -u) \$(id -g)"
    echo "After running, you should have a graalvm installation that can be used as your JAVA_HOME."
    echo "WARNING: you should not move the graalvm installation as it contains absolute links. Rather move the script to the desired directory before running."
    exit 1
fi

uid=$1
gid=$2

# Update package manager.
apt update

# We need wget.
apt install -y wget

# Download the jvm.
wget https://github.com/graalvm/graalvm-ce-builds/releases/download/jdk-17.0.7/graalvm-community-jdk-17.0.7_linux-x64_bin.tar.gz -O - | tar -vzxf -

# Install python and js support.
$GRAALVM/bin/gu install python js llvm

# Important to install pillow.
apt install -y \
    libtiff5-dev \
    libjpeg62-turbo-dev \
    libopenjp2-7-dev zlib1g-dev \
    libfreetype6-dev \
    liblcms2-dev \
    libwebp-dev \
    tcl8.6-dev \
    tk8.6-dev python3-tk \
    libharfbuzz-dev \
    libfribidi-dev \
    libxcb1-dev

# Load python virtual environment and its packages.
$GRAALVM/bin/graalpy -m venv $GRAALVM/hydra-venv
source $GRAALVM/hydra-venv/bin/activate
pip install numpy==1.23.5
pip install requests==2.32.3
pip install pillow==9.2.0

# Adjust permissions to the host user.
chown -R $uid:$gid $GRAALVM
