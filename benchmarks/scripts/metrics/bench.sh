#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)

EXPERIMENT_HOME="$DIR/experiments/$(date +%Y%m%d_%H%M%S)"

GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Set LIMIT_RESOURCES=true to change cpu and memory limits inside the containers 
LIMIT_RESOURCES=false

source $DIR/hydra_bench.sh
source $DIR/hydra_si_bench.sh
source $DIR/faastion_bench.sh

function print_tput {
    local log_dir=$1 c=$2
    tput=$(cat $log_dir/$c-ab*.log | grep 'Requests per second:' | awk '{sum += $4} END {print sum}')
    echo "Throughput is ~$tput req/s"
}

function log_resources {
    local log_dir=$1
    local c=$2
    mem_file=$log_dir/$c-mem.log
    cpu_file=$log_dir/$c-cpu_util.log
    
    while :
    do
        out=$(free -m --si)
        used_mem=$(echo "$out" | awk 'NR==2 {print $3}')
        out=$(cat /proc/stat)
        cpu_util=$(echo "$out" | head -n 1)
        echo $used_mem >> $mem_file
        echo $cpu_util >> $cpu_file
        sleep .100
    done    
}

function stop_containers {
    while docker container ps -a | grep sbox &> /dev/null; do # make sure containers have stopped
        docker container stop $(docker container ps -a | grep sbox | awk '{print $1}') &> /dev/null
    done
}

function run {
    local benchmark=$1
    local approach=$2
    log_dir=$EXPERIMENT_HOME/$benchmark/$approach/logs

    echo -e "${GREEN}## Measuring metrics for $approach - $benchmark ##${NC}"
    mkdir -p $log_dir

    for c in ${CONCURRENCY[@]}
    do
        launch_$approach $benchmark $c
        log_resources $log_dir $c &
        pid=$!
        benchmark_$approach $benchmark $log_dir $c
        kill $pid
        stop_containers
        sleep 2
        print_tput $log_dir $c
    done
}

function cleanup_resources {
    echo "Received signal, cleaning up resources..."
    stop_containers
    exit
}

if [ -z "${ARGO_HOME}" ]; then
    echo "ARGO_HOME is not defined. Exiting..."
    exit 1
fi

if [ -z "${WEBSERVER_IP}" ]; then
    echo "WEBSERVER_IP is not defined. Exiting..."
    exit 1
fi

trap 'cleanup_resources' SIGINT

# Comment/uncomment to add or remove benchmarks
BENCHMARKS+=(gv_bfs)
BENCHMARKS+=(gv_classify)
BENCHMARKS+=(gv_compression)
BENCHMARKS+=(gv_dna)
BENCHMARKS+=(gv_dynamic_html)
BENCHMARKS+=(gv_mst)
BENCHMARKS+=(gv_pagerank)
BENCHMARKS+=(gv_uploader)

# BENCHMARKS+=(gv_thumbnail) # FIXME: under high concurrency levels, platform crashes with segfault
# BENCHMARKS+=(gv_videoprocessing) # FIXME: under high concurrency levels, some threads will receive stop signal randomly

# Comment/uncomment to add or remove baselines
APPROACHES+=(faastion)
APPROACHES+=(hydra)
APPROACHES+=(hydra_si)

# Change to desired concurrency level
CONCURRENCY=(1 8 14 20)

for benchmark in ${BENCHMARKS[@]}
do

    # faastion can only execute 14 concurrent classify requests because of dlmopen limit
    if [ "$benchmark" = "gv_classify" ]; then
        CONCURRENCY=(1 8 14)
    fi

    for approach in ${APPROACHES[@]}
    do
        run $benchmark $approach
    done
done

$DIR/preprocess_data.sh $EXPERIMENT_HOME