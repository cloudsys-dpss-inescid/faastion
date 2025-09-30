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

function install_firecracker {
    wget https://github.com/firecracker-microvm/firecracker/releases/download/v1.1.0/firecracker-v1.1.0-x86_64.tgz
    tar -vzxf firecracker-v1.1.0-x86_64.tgz
    mv release-v1.1.0-x86_64 $ARGO_HOME/resources/firecracker-v1.1.0-x86_64
    ln -s $ARGO_HOME/resources/firecracker-v1.1.0-x86_64/firecracker-v1.1.0-x86_64 $ARGO_HOME/resources/firecracker-v1.1.0-x86_64/firecracker
    rm firecracker-v1.1.0-x86_64.tgz
}

function install_musl {
    # Option 1: Download a musl toolchain.
    function dist_musl {
        wget https://more.musl.cc/10/x86_64-linux-musl/x86_64-linux-musl-native.tgz
        tar -vzxf x86_64-linux-musl-native.tgz
        mv x86_64-linux-musl-native $ARGO_HOME/resources/x86_64-linux-musl-native
        rm x86_64-linux-musl-native.tgz
    }

    # Option 2: Use a local musl toolchain build.
    function custom_musl {
        # Step 1: clone the toolchain build project and change directory.
        git clone https://git.zv.io/toolchains/musl-cross-make.git $ARGO_HOME/resources/musl-cross-make
        cd $ARGO_HOME/resources/musl-cross-make
        # Step 2: download all the sources.
        make TARGET=x86_64-linux-musl sources
        # Step 3: replace musl sources with MUSL_HOME.
        rm -r sources/musl-1.2.3
        cp -r $MUSL_HOME sources/musl-1.2.3
        tar -vzcf sources/musl-1.2.3.tar.gz -C sources musl-1.2.3
        # Step 4: build. Use -j if you have a large number of cores.
        make -j52 TARGET=x86_64-linux-musl clean install | tee ~/make.log
        cd -
        ln -s $ARGO_HOME/resources/musl-cross-make/output $ARGO_HOME/resources/x86_64-linux-musl-native
    }

    if [ -z "$MUSL_HOME" ]
    then
        echo "Using musl distribution."
        dist_musl
    else
        echo "Using custom musl distribution ($MUSL_HOME)."
        custom_musl
    fi

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

# Preparing environment.
mv $ARGO_HOME/env.sh   $ARGO_HOME/env.sh.back   &> /dev/null
mv $ARGO_HOME/env.fish $ARGO_HOME/env.fish.back &> /dev/null
echo "export  ARGO_HOME=$ARGO_HOME"     >> $ARGO_HOME/env.sh
echo "set -gx ARGO_HOME $ARGO_HOME"     >> $ARGO_HOME/env.fish
echo "export  WORK_DIR=\$ARGO_HOME/tmp"  >> $ARGO_HOME/env.sh
echo "set -gx WORK_DIR \$ARGO_HOME/tmp"  >> $ARGO_HOME/env.fish
echo "export  PATH=\$ARGO_HOME/resources/firecracker-v1.1.0-x86_64:\$PATH"   >> $ARGO_HOME/env.sh
echo "set -gx PATH \$ARGO_HOME/resources/firecracker-v1.1.0-x86_64 \$PATH"   >> $ARGO_HOME/env.fish
echo "export  JAVA_HOME=\$ARGO_HOME/resources/graalvm-jdk-17.0.7+8.1"        >> $ARGO_HOME/env.sh
echo "set -gx JAVA_HOME \$ARGO_HOME/resources/graalvm-jdk-17.0.7+8.1"        >> $ARGO_HOME/env.fish
echo "export  HYDRA_PYTHON=" >> $ARGO_HOME/env.sh
echo "set -gx HYDRA_PYTHON" >> $ARGO_HOME/env.fish

if [ ! -f $JAVA_HOME/bin/java ];
then
    echo "JVM not found. Installing..."
    install_graalvm
fi

if [ ! -f $ARGO_HOME/resources/firecracker-v1.1.0-x86_64/firecracker ];
then
    echo "Firecracker not found. Installing..."
    install_firecracker
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

if ! command -v docker &> /dev/null
then
    echo "WARNING: docker could not be found!"
    echo "If you plan on experimenting with Graalvisor's container backend then you should install Docker."
    GRAALVISOR_BUILD_MODE="local"
fi

if [ ! -e $ARGO_HOME/benchmarks/.git ];
then
    echo "Cloning benchmarks git module..."
    git pull --recurse-submodules
    echo "Cloning benchmarks git module... done!"
fi

if [ ! -f $ARGO_HOME/resources/hello-vmlinux.bin ];
then
    echo "Downloading linux kernel image..."
    curl -fsSL -o $ARGO_HOME/resources/hello-vmlinux.bin https://s3.amazonaws.com/spec.ccfc.min/img/hello/kernel/hello-vmlinux.bin
    echo "Downloading linux kernel image... done!"
fi

read -p "Build lambda manager? (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    bash $ARGO_HOME/lambda-manager/build.sh
fi

read -p "Build builder container image? (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    bash $ARGO_HOME/builder/build.sh
fi

read -p "Build graalvisor? (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    bash $ARGO_HOME/graalvisor/build.sh $GRAALVISOR_BUILD_MODE
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

bash $ARGO_HOME/images/build.sh
bash $ARGO_HOME/benchmarks/scripts/build_benchmarks.sh
