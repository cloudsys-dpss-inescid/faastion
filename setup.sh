#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

function install_graalvm {
    wget https://download.oracle.com/graalvm/17/archive/graalvm-jdk-17.0.7_linux-x64_bin.tar.gz
    tar -vzxf graalvm-jdk-17.0.7_linux-x64_bin.tar.gz
    mv graalvm-jdk-17.0.7+8.1 $ARGO_HOME/resources
    rm graalvm-jdk-17.0.7_linux-x64_bin.tar.gz
}

function install_musl {

    function custom_musl {
        # Step 1: clone the toolchain build project and change directory.
        git clone https://git.zv.io/toolchains/musl-cross-make.git $ARGO_HOME/resources/musl-cross-make
        cd $ARGO_HOME/resources/musl-cross-make
        # Step 2: download all the sources.
        make TARGET=x86_64-linux-musl sources
        # Step 3: replace musl sources with our custom musl.
        rm -r sources/musl-1.2.3
        cp -r $ARGO_HOME/musl sources/musl-1.2.3
        tar -vzcf sources/musl-1.2.3.tar.gz -C sources musl-1.2.3
        # Step 4: build. Use -j if you have a large number of cores.
        make -j52 TARGET=x86_64-linux-musl clean install | tee ~/make.log
        cd -
        ln -s $ARGO_HOME/resources/musl-cross-make/output $ARGO_HOME/resources/x86_64-linux-musl-native
    }

    custom_musl

    wget https://zlib.net/current/zlib.tar.gz
    tar -vzxf zlib.tar.gz
    rm -rf $ARGO_HOME/resources/zlib-1.3.1 &> /dev/null
    mv zlib-1.3.1 $ARGO_HOME/resources/
    rm zlib.tar.gz

    CC=$ARGO_HOME/resources/x86_64-linux-musl-native/bin/x86_64-linux-musl-cc
    cd $ARGO_HOME/resources/zlib-1.3.1
    ./configure --prefix=$ARGO_HOME/resources/x86_64-linux-musl-native/x86_64-linux-musl --static
    make
    make install
    cd - &> /dev/null
}

export ARGO_HOME=$(DIR)
export WORK_DIR=$ARGO_HOME/tmp
export JAVA_HOME=$ARGO_HOME/resources/graalvm-jdk-17.0.7+8.1

# Creare resources directory.
mkdir $ARGO_HOME/resources &> /dev/null

# Preparing environment.
mv $ARGO_HOME/env.sh   $ARGO_HOME/env.sh.back   &> /dev/null
mv $ARGO_HOME/env.fish $ARGO_HOME/env.fish.back &> /dev/null
echo "export  ARGO_HOME=$ARGO_HOME"     >> $ARGO_HOME/env.sh
echo "set -gx ARGO_HOME $ARGO_HOME"     >> $ARGO_HOME/env.fish
echo "export  WORK_DIR=\$ARGO_HOME/tmp"  >> $ARGO_HOME/env.sh
echo "set -gx WORK_DIR \$ARGO_HOME/tmp"  >> $ARGO_HOME/env.fish
echo "export  JAVA_HOME=\$ARGO_HOME/resources/graalvm-jdk-17.0.7+8.1"        >> $ARGO_HOME/env.sh
echo "set -gx JAVA_HOME \$ARGO_HOME/resources/graalvm-jdk-17.0.7+8.1"        >> $ARGO_HOME/env.fish

if [ ! -f $JAVA_HOME/bin/java ];
then
    echo "JVM not found. Installing..."
    install_graalvm
fi

if [ ! -e $ARGO_HOME/resources/x86_64-linux-musl-native/ ];
then
    read -p "Musl not found. Install musl in $ARGO_HOME/resources? (y or Y, everything else as no)? " -n 1 -r
    echo    # move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
        install_musl
    fi
fi

read -p "Build graalvisor? (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    bash $ARGO_HOME/graalvisor/build.sh
fi

read -p "Build javassist transformer? (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    bash $ARGO_HOME/native-execution/javassist/build.sh
fi

read -p "Build graalvisor test (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    bash $ARGO_HOME/benchmarks/src/java/gv-hello-world/build_script.sh build_ni_sharedlibrary
    echo "Now you can try running a graalvisor hello world by running:"
    echo "> export WORK_DIR=$ARGO_HOME/tmp"
    echo "> sudo -E $ARGO_HOME/benchmarks/scripts/benchmark-graalvisor.sh svm gv_java_hw test 1"
fi

# TODO - build javassist?
