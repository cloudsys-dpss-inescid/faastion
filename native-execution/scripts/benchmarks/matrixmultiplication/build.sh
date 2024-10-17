#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

function build_jni_libraries {
        SRC_DIR="src/main/c"
        LIB_DIR="build/libs"
        JNI_INCLUDE="-I$JAVA_HOME/include -I$JAVA_HOME/include/linux"
        CFLAGS="-Wall -g -fPIC -shared $JNI_INCLUDE"

        gcc $CFLAGS -o $LIB_DIR/libmatmul.so $SRC_DIR/MatrixMultiplication.c
}

function run_jni {

        LIB_DIR="$(DIR)/build/libs"
        CLS_DIR="build/classes/java/main"
        java -cp $CLS_DIR -Djava.library.path=$LIB_DIR MatrixMultiplication

}

./gradlew clean shadowJar assemble

export LD_LIBRARY_PATH=$(DIR)/build/libs:$LD_LIBRARY_PATH
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

build_jni_libraries
run_jni

export class_path=$(DIR)/build/libs/matmul-all.jar
export entrypoint="MatrixMultiplication"
