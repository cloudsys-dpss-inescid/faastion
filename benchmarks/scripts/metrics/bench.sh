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

function log_resources {
    local log_dir=$1
    local c=$2
    mem_file=$log_dir/$c-mem.log
    cpu_file=$log_dir/$c-cpu_util.log

    rm -f $mem_file $cpu_file
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

# If the benchmark reaches a certain time limit, then stop all containers and retry
function health_check {
    sleep 300 && stop_containers
}

function run_attempt {
    local benchmark=$1 approach=$2 log_dir=$3 c=$4

    echo "Running $c parallel requests..."

    for try in $(seq 1 5)
    do
	if [ $try -gt 1 ]; then
            rm -f $log_dir/$c-ab*.log
            echo "Retrying"
	fi

	# Get CPU utilization and memory footprint while idle
	log_resources $log_dir $c &
        log_pid=$!
	sleep 2

	# Launch and register functions
        launch_$approach $benchmark $c

	# Monitor for system hangs
	health_check &
	hc_pid=$!

	# Run benchmark
        benchmark_$approach $benchmark $log_dir $c

	# Teardown
	kill $log_pid
	sleep_pid=$(ps --ppid $hc_pid | awk 'NR==2{print $1}')
	kill $hc_pid
	(kill $sleep_pid &> /dev/null)
        stop_containers
        sleep 2

	# Return if no problems were found
	tput=$(tput_$approach $benchmark $log_dir $c)
        if [ "$tput" ]; then
            echo "Throughput is ~$tput req/s"
            return
        fi
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
        run_attempt $benchmark $approach $log_dir $c
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
CONCURRENCY=(1 8 14 20 32 48 64)

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
