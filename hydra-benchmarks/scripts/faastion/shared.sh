#!/bin/bash

# Function to register Java applications in GraalVM
function gv_java_native_hw_n {
    APP_LANG=java
    APP_NAME=gv-native-hw-$1
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libnativehw$1.so
    APP_MAIN=com.jni.HelloJNI
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8081/register?name=nativehw$1\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"nativehw'$1'","async":"false","cached":"true","arguments":""}' > /tmp/payload$1.post
}

function gv_java_native_factors {
    APP_LANG=java
    APP_NAME=gv-native-factorization
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libfactors.so
    APP_MAIN=com.jni.Factorization
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8081/register?name=factors\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"factors","async":"false","cached":"true","arguments":""}' > /tmp/payload.post
}

function gv_java_native_matrix {
    APP_LANG=java
    APP_NAME=gv-native-matrixmul
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libmatrixmul.so
    APP_MAIN=com.jni.MatrixMultiplication
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8081/register?name=matrixmul\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"matrixmul","async":"false","cached":"true","arguments":""}' > /tmp/payload.post
}

# Function to start the Graalvisor
function start_svm {
    export lambda_timestamp="$(date +%s%N | cut -b1-13)"
    export lambda_port="8081"
    export LD_LIBRARY_PATH=$ARGO_HOME/graalvisor/build/libs:$LD_LIBRARY_PATH
    export LD_PRELOAD=$ARGO_HOME/graalvisor/build/libs/libpreload.so
    # Start Graalvisor
    $ARGO_HOME/graalvisor/build/native-image/polyglot-proxy &
    wait
}

function warmup {
    # Sequential
    for j in $(seq $1)
    do
        curl -s -X POST 127.0.0.1:8081 -H 'Content-Type: application/json' -d $(cat /tmp/payload.post) >/dev/null
    done

    # Concurrent
    (
        for j in $(seq 3)
        do
            curl -s -X POST 127.0.0.1:8081 -H 'Content-Type: application/json' -d $(cat /tmp/payload.post) &>/dev/null &
        done
        wait
    )
}