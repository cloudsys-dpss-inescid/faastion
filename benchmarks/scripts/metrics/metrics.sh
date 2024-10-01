#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

GRAALVISOR_HOME=$ARGO_HOME/graalvisor
BENCHMARKS_HOME=$ARGO_HOME/benchmarks
JAVA_BENCHMARKS=$BENCHMARKS_HOME/src/java

experiment_name=$(date +"experiment_%Y%m%d_%H%M%S")

LOGS_HOME=$(DIR)/logs/$experiment_name

GREEN='\033[0;32m'
NC='\033[0m' # No Color

function gv_java_native_factors {
    APP_LANG=java
    APP_NAME=gv-native-factorization
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libfactors.so
    APP_MAIN=com.jni.Factorization
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=factors\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"factors","async":"false","cached":"true","arguments":""}' > /tmp/payload1.post
}

function gv_java_native_matmul {
    APP_LANG=java
    APP_NAME=gv-native-matmul
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libmatmul.so
    APP_MAIN=com.jni.MatrixMultiplication
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=matmul\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"matmul","async":"false","cached":"true","arguments":""}' > /tmp/payload2.post
}

function gv_java_httprequest {
    APP_LANG=java
    APP_NAME=gv-httprequest
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libhttprequest.so
    APP_MAIN=com.httprequest.HttpRequest
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=httprequest\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"httprequest","async":"false","cached":"true","arguments":""}' > /tmp/payload3.post
}

function gv_java_sleep {
    APP_LANG=java
    APP_NAME=gv-sleep
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libsleep.so
    APP_MAIN=com.sleep.Sleep
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=sleep\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"sleep","async":"false","cached":"true","arguments":""}' > /tmp/payload4.post
}

function gv_java_native_hw {
    APP_LANG=java
    APP_NAME=gv-native-hw
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libnativehw.so
    APP_MAIN=com.jni.HelloJNI
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=nativehw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"nativehw","async":"false","cached":"true","arguments":""}' > /tmp/payload5.post
}

function gv_java_hw {
    APP_LANG=java
    APP_NAME=gv-hello-world
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libhelloworld.so
    APP_MAIN=com.hello_world.HelloWorld
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=hw\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"hw","async":"false","cached":"true","arguments":""}' > /tmp/payload6.post
}

function gv_java_maxtrixmul {
    APP_LANG=java
    APP_NAME=gv-matrixmul
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libmanmatrixmul.so
    APP_MAIN=com.matrix_mul.MatrixMul
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=manmatrixmul\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"manmatrixmul","async":"false","cached":"true","arguments":""}' > /tmp/payload1.post
}

function gv_java_factors {
    APP_LANG=java
    APP_NAME=gv-factorization
    APP_SO=$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/libmanfactors.so
    APP_MAIN=com.factorization.Factorization
    # Register the native application in GraalVM
    curl -s -X POST 127.0.0.1:8080/register?name=manfactors\&entryPoint=$APP_MAIN\&language=$APP_LANG\&sandbox=$SANDBOX -H 'Content-Type: application/json' --data-binary @$APP_SO
    echo '{"name":"manfactors","async":"false","cached":"true","arguments":""}' > /tmp/payload1.post
}

function compile_jni_benchmarks {
    # for benchmark in gv-native-hw gv-native-factorization gv-native-matmul
    for benchmark in gv-native-hw
    do
        cd "$JAVA_BENCHMARKS/$benchmark"
        ./build_script.sh
        cd -
    done
}

function compile_benchmarks {
    for benchmark in gv-native-factorization gv-native-hw gv-native-matmul
    do
        cd "$JAVA_BENCHMARKS/$benchmark"
        ./build_script_proc_iso.sh
        cd -
    done

    for benchmark in gv-hello-world gv-httprequest gv-factorization
    do
        cd "$JAVA_BENCHMARKS/$benchmark"
        ./build_script.sh
        cd -
    done
}

function start_svm {
    export lambda_timestamp="$(date +%s%N | cut -b1-13)"
    export lambda_port="8080"
    export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:$LD_LIBRARY_PATH
    # export LD_PRELOAD=$GRAALVISOR_HOME/build/libs/libpreload.so
    # Start Graalvisor
    $GRAALVISOR_HOME/build/native-image/polyglot-proxy &
    wait
    # unset LD_PRELOAD
}

function log_resources {
    PID=$1
    OFILE_RSS=/tmp/$experiment_name/$approach/footprint.csv

    rm $OFILE_RSS &> /dev/null
        while kill -0 $PID &> /dev/null; do
                # The idea for memory is that we traverse the entire pid subprocess tree
                # and memory memory utilization. We sum all individual memory and return.
                s_mem=0
                timestamp=$(date -u +"%s%3N")
                for p in $(pstree -p $PID | grep -o '([0-9]\+)' | grep -o '[0-9]\+')
                do
                    p_mem=$(ps -q $p -o rss=)
                    s_mem=$((s_mem + p_mem))
                done
                echo "$s_mem,$timestamp" >> $OFILE_RSS
                sleep .100
        done
}

function benchmark {
    wrk -t$WORKLOAD -c$WORKLOAD -d$DURATION -s native.lua http://127.0.0.1:8080

    # Kill Graalvisor
    pkill -9 -f polyglot-proxy
}

function capture {
    # Execute wrk and capture the output
    output=$(benchmark)

    # Extract the average latency
    avg_latency=$(echo "$output" | grep "Latency" | awk '{print $2}')

    # Extract the standard deviation of latency
    stddev_latency=$(echo "$output" | grep "Latency" | awk '{print $3}')

    # Extract the throughput (requests per second)
    throughput=$(echo "$output" | grep "Requests/sec" | awk '{print $2}')

    echo $avg_latency > /tmp/$experiment_name/$approach/avg_latency.log
    echo -e "Latency Stdev: $stddev_latency, Throughput: $throughput" > $LOGS_HOME/$approach-data.txt

    for function in factors matmul sleep httprequest nativehw hw
    do
        function_responses=$(echo $output | tr ' ' '\n' | tr '{' '\n' | tr -d '}' | grep $function)
        echo "$function_responses" | while read -r line
        do
            process_time=$(echo "$line" | awk -F'"process_time\\(us\\)":' '{print $2}')
            echo "$process_time" >> "/tmp/$experiment_name/$approach/$function-latency.log"
        done
    done
}

function warmup {
    # Sequential
    for i in $(seq 6)
    do
        curl -s -X POST 127.0.0.1:8080 -H 'Content-Type: application/json' -d $(cat /tmp/payload$i.post) &>/dev/null 
    done

    # Concurrent
    (
        for i in $(seq 6)
        do
            curl -s -X POST 127.0.0.1:8080 -H 'Content-Type: application/json' -d $(cat /tmp/payload$i.post) &>/dev/null &
        done
        wait
    )
}

function register {
    # gv_java_native_factors
    # gv_java_factors
    # gv_java_native_matmul
    # gv_java_httprequest
    # gv_java_sleep
    gv_java_native_hw
    # gv_java_hw
    # gv_java_maxtrixmul
}

function LPI_requirements {
    mkdir -p /tmp/fifo
    mkdir -p /tmp/ret
}

function LPI_cleanup {
    rm -rf /tmp/ret
    rm -rf /tmp/fifo
}

function execute {
    
    # Start Graalvisor
    start_svm &> $LOGS_HOME/$approach-lambda.log &
    PID=$(echo -n "$!")

    # Log Resources (memory and CPU)
    log_resources $PID &

    # Register applications
    register

    # Comment for cold-start measures
    # warmup
    
    # Run Benchmarking tool
    #capture
    benchmark
}

function setup {
    base_dir="/tmp/${experiment_name}"
    # directories=("isolate" "process" "faastlane" "faastion")
    directories=("isolate" "process" "faastion")
    for dir in "${directories[@]}"; do
        mkdir -p "${base_dir}/${dir}"
    done
}

function start_webserver {
    cd $(DIR)/webserver
    $(DIR)/webserver.sh &> $LOGS_HOME/webserver.log &
    cd -
}

function cleanup_resources {
    echo "Received signal, cleaning up resources..."
    pkill -9 -f polyglot-proxy
    # LPI_cleanup
    exit
}

# Preparing global paths.
if [ -z "${ARGO_HOME}" ]; then
    echo "ARGO_HOME is not defined. Existing..."
    exit 1
fi

# Clean resources if killed with signal
trap 'cleanup_resources' SIGINT

WORKLOAD=1
DURATION="1s"

echo "$experiment_name" > /tmp/experiment_name.log

mkdir -p $LOGS_HOME

# LPI_requirements

setup

### SCRIPT STARTS HERE ###
# start_webserver

# compile_benchmarks

# for approach in isolate process
# do
#     echo -e "${GREEN}###################################################"
#     echo -e "          Measuring metrics for $approach         "
#     echo -e "###################################################${NC}"

#     export SANDBOX=$approach
#     execute
#     sleep 1
# done

# # JNI compilation
# compile_jni_benchmarks

export SANDBOX=isolate
for approach in faastion # faastlane 
do
    echo -e "${GREEN}###################################################"
    echo -e "          Measuring metrics for $approach         "
    echo -e "###################################################${NC}"

    [ "$approach" = "faastlane" ] && export EAGER_MPK=1
    execute
    sleep 1
    [ "$approach" = "faastlane" ] && unset EAGER_MPK
done


# Generate plots
# python3 metrics.py
echo "Experiment: $experiment_name"


# Clean resources when finished
cleanup_resources
