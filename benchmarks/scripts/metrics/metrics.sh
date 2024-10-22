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
    base_url="127.0.0.1:8080/register?entryPoint=$APP_MAIN&language=$APP_LANG"

    if [ "$approach" = "faastion" ] || [ "$approach" = "faastion_lpi" ]; then
        for idx in $(seq 1 $WORKLOAD); do
            curl -s -X POST "$base_url&name=$LIB_NAME$idx&sandbox=$SANDBOX" -H "$headers" \
                --data-binary @"$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/lib$LIB_NAME$idx.so" &> /dev/null
        done
    else
        curl -s -X POST "$base_url&name=$LIB_NAME&sandbox=$SANDBOX" -H "$headers" \
            --data-binary @"$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/lib$LIB_NAME.so" &> /dev/null
    fi

    if [ "$approach" = "faastion_lpi" ]; then
        curl -s -X POST "$base_url&name=$LIB_NAME&sandbox=process" -H "$headers" \
            --data-binary @"$ARGO_HOME/benchmarks/src/$APP_LANG/$APP_NAME/build/lib$LIB_NAME.so" &> /dev/null
    fi
}

function register_gv_aes_encryption {
    APP_LANG=java
    APP_NAME=gv-aes-encryption
    APP_MAIN=com.jni.AESEncryption

    LIB_NAME="aes"

    register_function
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

function register_gv_factorization {
    APP_LANG=java
    APP_NAME=gv-factorization
    APP_MAIN=com.factorization.Factorization

    LIB_NAME="manfactors"

    register_function
}

function register_gv_native_matmul {
    APP_LANG=java
    APP_NAME=gv-native-matmul
    APP_MAIN=com.jni.MatrixMultiplication

    LIB_NAME="matmul"

    register_function
}

function register_gv_httprequest {
    APP_LANG=java
    APP_NAME=gv-httprequest
    APP_MAIN=com.httprequest.HttpRequest

    LIB_NAME="httprequest"

    register_function
}

function register_gv_matrixmul {
    APP_LANG=java
    APP_NAME=gv-matrixmul
    APP_MAIN=com.matrix_mul.MatrixMul

    LIB_NAME="manmatrixmatmul"

    register_function
}

function register_gv_native_hw {
    APP_LANG=java
    APP_NAME=gv-native-hw
    APP_MAIN=com.jni.HelloJNI

    LIB_NAME="nativehw"

    register_function
}

function register_gv_hello_world {
    APP_LANG=java
    APP_NAME=gv-hello-world
    APP_MAIN=com.hello_world.HelloWorld

    LIB_NAME="helloworld"

    register_function
}

function start_svm {
    export lambda_timestamp="$(date +%s%N | cut -b1-13)"
    export lambda_port="8080"
    export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:$LD_LIBRARY_PATH
    # export LD_PRELOAD=$GRAALVISOR_HOME/build/libs/libpreload.so
    # Start Graalvisor
    GLIBC_TUNABLES="glibc.rtld.nns=16" $GRAALVISOR_HOME/build/native-image/polyglot-proxy &> /dev/null &
    PID=$!
    # unset LD_PRELOAD
}

function log_resources {
    OFILE_RSS=$RESULTS_HOME/$approach/memory/$WORKLOAD-footprint.csv
    rm $OFILE_RSS &> /dev/null
    while kill -0 $PID &> /dev/null; do
        timestamp=$(date -u +"%s%3N")
        s_mem=$(ps -p $PID --ppid $PID -o rss= | awk '{sum+=$1} END {print sum}')
        echo "$s_mem,$timestamp" >> $OFILE_RSS
        sleep .100
    done
}

function run_wrk {
    if [ "$approach" = "faastion" ] || [ "$approach" = "faastion_lpi" ]; then
        script="faastion.lua"
    else
        script="native.lua"
    fi

    env function_name=$LIB_NAME wrk --latency -t$WORKLOAD -c$WORKLOAD -d$DURATION -s $script http://127.0.0.1:8080 &> "$output"
}

function benchmark {
    output="$RESULTS_HOME/$approach/debug/$WORKLOAD-wrk_output.txt"
    DURATION="${wrk_duration[$BENCH_ID]}s"
    run_wrk $WORKLOAD $DURATION

    # Kill Graalvisor
    pkill -9 -f polyglot-proxy
}

function warmup {
    output="/dev/null"
    DURATION="${warmup_duration[$BENCH_ID]}s"
    run_wrk
}

function capture {
    # Execute wrk and capture the output
    benchmark

    # Extract percentiles
    top50=$(less $output | grep "50%" | awk 'END {print $2}')
    top75=$(less $output | grep "75%" | awk 'END {print $2}')
    top90=$(less $output | grep "90%" | awk 'END {print $2}')
    top99=$(less $output | grep "99%" | awk 'END {print $2}')

    # Extract the average latency
    avg_latency=$(less $output | grep "Latency" | awk 'NR==1 {print $2}')

    # Extract the standard deviation of latency
    stddev_latency=$(less $output | grep "Latency" | awk 'NR==1 {print $3}')

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

    mv domain_usage.txt     $RESULTS_HOME/$approach/domain_usage/$WORKLOAD-domains.txt
}

function execute {
    
    # Start Graalvisor
    start_svm

    # Log Resources (memory and CPU)
    log_resources &

    # Register applications
    register_$benchmark_name

    warmup
    
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

function execute_faastion_lpi {
    export LPI=true
    execute
    unset LPI
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

    directories=("isolate" "process" "faastlane" "faastion" "faastion_lpi")
    for dir in "${directories[@]}"; do
        mkdir -p "${RESULTS_HOME}/${dir}/debug" "${RESULTS_HOME}/${dir}/memory" "${RESULTS_HOME}/${dir}/latency" "${RESULTS_HOME}/${dir}/domain_usage"
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

workloads=(1 2 4 8 16 32 48 64)
benchmarks=(gv_native_hw gv_native_factors gv_filehashing gv_aes_encryption gv_hello_world gv_httprequest gv_matrixmul gv_factorization gv_native_matmul)

warmup_duration=( 1 10 10 10 1 2 1 10 1)
wrk_duration=(    3 50 50 50 3 8 3 50 3)
total_duration=0
for i in ${!wrk_duration[@]}
do
    time_warmup=${warmup_duration[$i]}
    time_wrk=${wrk_duration[$i]}
    total_duration=$(echo "scale=4; $total_duration + (($time_warmup + $time_wrk + 1) * ${#workloads[@]} * 5 / 60)" | bc)
done
echo "Estimated benchmarks time ~= $total_duration mins"

for BENCH_ID in ${!benchmarks[@]}
do
    benchmark_name="${benchmarks[$BENCH_ID]}"
    setup
    for WORKLOAD in "${workloads[@]}"
    do
        for approach in isolate faastlane faastion faastion_lpi process
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

# Clean resources when finished
cleanup_resources
