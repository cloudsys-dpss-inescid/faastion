#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

GRAALVISOR_HOME=$ARGO_HOME/graalvisor
BENCHMARKS_HOME=$ARGO_HOME/benchmarks
JAVA_BENCHMARKS=$BENCHMARKS_HOME/src/java

EXPERIMENT_HOME="$(DIR)/experiments/$(date +%Y%m%d_%H%M%S)"

GREEN='\033[0;32m'
NC='\033[0m' # No Color

function register_function {
    headers='Content-Type: application/json'
    base_url="127.0.0.1:8080/register?entryPoint=$APP_MAIN&language=$APP_LANG&sandbox=$SANDBOX"

    if [ "$approach" != "faastion" ]; then
        curl -s -X POST "$base_url&name=$LIB_NAME" -H "$headers" \
            --data-binary @"$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/lib$LIB_NAME.so" &> /dev/null
    else
        for idx in $(seq 1 $WORKLOAD); do
            curl -s -X POST "$base_url&name=$LIB_NAME$idx" -H "$headers" \
                --data-binary @"$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/lib$LIB_NAME$idx.so" &> /dev/null
        done
    fi
}

function register_gv_filehashing {
    APP_LANG=java
    APP_NAME=gv-file-hashing
    APP_MAIN=com.jni.FileHashing

    LIB_NAME="filehashing"

    register_function
}

function register_gv_native_factors {
    APP_LANG=java
    APP_NAME=gv-native-factorization
    APP_MAIN=com.jni.Factorization

    LIB_NAME="factors"

    register_function
}

function register_gv_native_matmul {
    APP_LANG=java
    APP_NAME=gv-native-matmul
    APP_MAIN=com.jni.MatrixMultiplication

    LIB_NAME="matmul"

    register_function
}

function register_gv_native_hw {
    APP_LANG=java
    APP_NAME=gv-native-hw
    APP_MAIN=com.jni.HelloJNI

    LIB_NAME="nativehw"

    register_function
}

function start_svm {
    export lambda_timestamp="$(date +%s%N | cut -b1-13)"
    export lambda_port="8080"
    export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:$LD_LIBRARY_PATH
    # export LD_PRELOAD=$GRAALVISOR_HOME/build/libs/libpreload.so
    # Start Graalvisor
    GLIBC_TUNABLES="glibc.rtld.nns=16" $GRAALVISOR_HOME/build/native-image/polyglot-proxy &
    wait
    # unset LD_PRELOAD
}

function log_resources {
    PID=$1
    OFILE_RSS=$RESULTS_HOME/$approach/memory/$WORKLOAD-footprint.csv

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
    
    if [ "$approach" = "faastion" ]; then
        script="faastion.lua"
    else
        script="native.lua"
    fi

    env function_name=$LIB_NAME wrk --latency -t$WORKLOAD -c$WORKLOAD -d$DURATION -s $script http://127.0.0.1:8080 &> "$output"

    # Kill Graalvisor
    pkill -9 -f polyglot-proxy
}

function capture {
    output="$RESULTS_HOME/$approach/debug/$WORKLOAD-wrk_output.txt"
    # Execute wrk and capture the output
    benchmark

    # Extract percentiles
    top50=$(less $output | grep "50%" | awk 'END {print $2}')
    top75=$(less $output | grep "75%" | awk 'END {print $2}')
    top90=$(less $output | grep "90%" | awk 'END {print $2}')
    top99=$(less $output | grep "99%" | awk 'END {print $2}')

    # Extract the average latency
    avg_latency=$(less $output | grep "Latency" | awk '{print $2}')

    # Extract the standard deviation of latency
    stddev_latency=$(less $output | grep "Latency" | awk '{print $3}')

    # Extract the throughput (requests per second)
    throughput=$(less $output | grep "Requests/sec" | awk '{print $2}')

    latency_home="$RESULTS_HOME/$approach/latency"

    echo $avg_latency       >> "$latency_home/avg_latency.txt"
    echo $stddev_latency    >> "$latency_home/stddev_latency.txt"
    echo $throughput        >> "$latency_home/throughput.txt"
    echo $top50             >> "$latency_home/50p.txt"
    echo $top75             >> "$latency_home/75p.txt"
    echo $top90             >> "$latency_home/90p.txt"
    echo $top99             >> "$latency_home/99p.txt"
}

function execute {
    
    # Start Graalvisor
    start_svm &> "$LOGS_HOME/$approach/$WORKLOAD-lambda.log" &
    PID=$(echo -n "$!")

    # Log Resources (memory and CPU)
    log_resources $PID &

    # Register applications
    register_$benchmark_name
    
    # Run Benchmarking tool
    capture
    # benchmark
}

function execute_isolate {
    execute
}

function execute_faastion {
    execute
}

function execute_faastlane {
    export faastlane=true
    execute
    unset faastlane
}

function execute_process {
    sb=$SANDBOX
    export SANDBOX=process
    execute
    export SANDBOX=$sb
}

function setup {
    echo "Running workloads for $benchmark_name"

    LOGS_HOME=$EXPERIMENT_HOME/$benchmark_name/logs
    RESULTS_HOME=$EXPERIMENT_HOME/$benchmark_name/results

    directories=("isolate" "process" "faastlane" "faastion")
    for dir in "${directories[@]}"; do
        mkdir -p "${RESULTS_HOME}/${dir}/debug" "${RESULTS_HOME}/${dir}/memory" "${RESULTS_HOME}/${dir}/latency" "${LOGS_HOME}/${dir}"
    done
}

function start_webserver {
    cd $(DIR)/webserver
    $(DIR)/webserver.sh &> /dev/null &
    cd -
}

function cleanup_resources {
    echo "Received signal, cleaning up resources..."
    pkill -9 -f polyglot-proxy
    exit
}

# Preparing global paths.
if [ -z "${ARGO_HOME}" ]; then
    echo "ARGO_HOME is not defined. Existing..."
    exit 1
fi

# Clean resources if killed with signal
trap 'cleanup_resources' SIGINT

### SCRIPT STARTS HERE ###
# start_webserver

export SANDBOX=isolate

workloads=(1 2 4 8 16 32)
for benchmark_name in gv_native_hw gv_native_matmul gv_native_factors gv_filehashing
do
    setup

    if [ "$benchmark_name" = "gv_native_factors" ]; then
        DURATION="10s"
    elif [ "$benchmark_name" = "gv_filehashing" ]; then
        DURATION="15s"
    else
        DURATION="1s"
    fi

    for WORKLOAD in "${workloads[@]}"
    do
        for approach in isolate faastlane faastion process
        do
            echo -e "${GREEN}###################################################"
            echo -e "       Measuring metrics for $approach - $WORKLOAD      "
            echo -e "###################################################${NC}"
            execute_$approach
            sleep 1
        done
    done
done


# Generate plots
# python3 metrics.py
echo "Experiment: $experiment_name"


# Clean resources when finished
cleanup_resources
