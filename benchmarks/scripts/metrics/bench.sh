#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)

EXPERIMENT_HOME="$DIR/experiments/$(date +%Y%m%d_%H%M%S)"

GREEN='\033[0;32m'
NC='\033[0m' # No Color

function stop_containers {
    while docker container ps -a | grep box &> /dev/null; do # make sure containers have stopped
        docker container stop $(docker container ps -a | grep box | awk '{print $1}') &> /dev/null
    done
}

function get_knative_image {
    IMG=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.kn_image')
    
    KN_DIR=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.kn_dir')
    SHARE_VOLUME=""
    if [ "$KN_DIR" != "null" ]; then
        SHARE_VOLUME="-v $HYDRA_BENCHMARKS_HOME/src/java/$KN_DIR/src/main/resources/native-image:/native-image"
    fi 
}

function start_multiple_containers {
    cpus=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.cpus')
    memory=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.memory')

    RESOURCES=""
    if [ "$alloc_resources" = "true" ]; then
        RESOURCES="--cpus=\"$cpus\" --memory=\"{$memory}m\""
    fi

    get_knative_image
    for i in $(seq 1 $workload)
    do
        port=$((8080 + $i))
        if [ "$approach" = "wsk" ]; then
            docker run -d --rm -p $port:8080 $RESOURCES $SHARE_VOLUME -v /lib:/lib --name sbox$i $IMG
        elif [ "$approach" = "hydra-si" ]; then
            docker run -d --rm --network host $RESOURCES -e lambda_port=$port --name sbox$i graalvisor
        else
            echo "ERROR: invalid baseline `$approach`"
            exit 1
        fi

        while ! nc -z localhost $port; do sleep 0.01; done
        sleep 2
    done
}

function start_single_container {
    cpus=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.cpus')
    memory=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.memory')
    active_wait=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.active_wait')
    cpu_count=$(echo "$workload * $cpus" | bc)
    guest_memory=$(echo "$workload * $memory" | bc)

    args=""
    if [ "$approach" = "faastion_lpi" ]; then
        args="--enable-lpi"
    elif [ "$approach" = "faastlane" ]; then
        if [ $workload -gt 15 ]; then
            cpu_count=$(echo "15 * 0.5" | bc)
            guest_memory=$(echo "15 * 512" | bc)
        fi
        args="--enable-early-booking"
    fi

    ACTIVE_WAIT_ENV=""
    if [ "$active_wait" != "null" ]; then
        ACTIVE_WAIT_ENV="-e ACTIVE_WAIT_CAP=$active_wait"
    fi

    RESOURCES=""
    if [ "$alloc_resources" = "true" ]; then
        RESOURCES="--cpus=\"$cpu_count\" --memory=\"{$guest_memory}m\""
    fi

    if [ "$approach" = "knative" ]; then
        get_knative_image
        docker run -d --rm -p 8080:8080 $RESOURCES $SHARE_VOLUME -v /lib:/lib --name sbox $IMG
    else
        docker run -d --rm --network host $ACTIVE_WAIT_ENV $RESOURCES --name sbox graalvisor $args
    fi
    
    while ! nc -z localhost 8080; do sleep 0.01; done
    sleep 2
}

function register_lambda {
    if [ "$approach" = "knative" ] || [ "$approach" = "wsk" ]; then
        post_body=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.kn_input')
        return 0
    fi

    headers='Content-Type: application/json'
    entrypoint=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.entrypoint')
    base_url="127.0.0.1:8080/register?entryPoint=$entrypoint&language=java"
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')
    dir=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.dir')
    
    sandbox="isolate"
    if [ "$approach" = "process" ]; then
        sandbox="process"
    fi

    if [ "$approach" = "faastion" ] || [ "$approach" = "faastion_lpi" ]; then
        lib_name="lib$name.so"
    else
        lib_name="lib${name}_vanilla.so"
    fi

    if [ "$approach" = "hydra-si" ]; then
        for i in $(seq 1 $workload)
        do
            port=$((8080 + $i))
            curl -s -X POST "127.0.0.1:$port/register?entryPoint=$entrypoint&language=java&name=${name}&sandbox=$sandbox" -H "$headers" \
                --data-binary @"$ARGO_HOME/benchmarks/src/java/graalvisor-runtime/$dir/build/$lib_name" &> /dev/null
        done
        return 0
    fi

    for i in $(seq 1 $workload)
    do
        curl -s -X POST "$base_url&name=${name}${i}&sandbox=$sandbox" -H "$headers" \
            --data-binary @"$ARGO_HOME/benchmarks/src/java/graalvisor-runtime/$dir/build/$lib_name" &> /dev/null
    done

    if [ $workload -gt 14 ] && [ "$approach" = "faastion_lpi" ]; then
        curl -s -X POST "$base_url&name=${name}-proc&sandbox=process" -H "$headers" \
            --data-binary @"$ARGO_HOME/benchmarks/src/java/graalvisor-runtime/$dir/build/lib${name}_vanilla.so" &> /dev/null
    fi
}

function run_knative {
    env body="$post_body" wrk --latency -t$workload -c$workload -d$duration -T$duration -s $DIR/knative.lua http://127.0.0.1:8080 &> "$output"
}

function run_wsk {
    for i in $(seq 1 $workload)
    do
        if [ "$is_warmup" != "true" ]; then
            output="$EXPERIMENT_HOME/$benchmark/$approach/debug/$workload-ab_output-$i.txt"
        fi
        port=$((8080 + $i))
        env body="$post_body" wrk --latency -t1 -c1 -d$duration -T$duration -s $DIR/knative.lua http://127.0.0.1:$port &> "$output" &
        # ab -k -l -p $DIR/post.json -T application/json -c 1 -n 100 localhost:$port/ &> "$output" &
    done
    while ps | grep wrk &> /dev/null; do sleep 1; done
}

function run_graalvisor {
    script=$DIR/faastion.lua
    if [ "$approach" = "faastlane" ]; then
        script=$DIR/faastlane.lua
    fi
    env function_name=$name wrk --latency -t$workload -c$workload -d$duration -T$duration -s $script http://127.0.0.1:8080 &> "$output"
}

function run_hydra_si {
    for i in $(seq 1 $workload)
    do
        if [ "$is_warmup" != "true" ]; then
            output="$EXPERIMENT_HOME/$benchmark/$approach/debug/$workload-wrk_output-$i.txt"
        fi
        port=$((8080 + $i))
        env function_name=$name wrk --latency -t1 -c1 -d$duration -T$duration -s $DIR/native.lua http://127.0.0.1:$port &> "$output" &
    done
    while ps | grep wrk &> /dev/null; do sleep 1; done
}

function warmup_classify {
    if [ "$benchmark" != "gv_classify" ]; then
        return 0
    fi

    if [ "$approach" = "knative" ]; then
        curl -s -X POST localhost:8080 -H 'Content-Type: application/json' --data-binary $post_body
    elif [ "$approach" != "wsk" ]; then
        for i in $(seq 1 14)
        do
            if [ $i -gt $workload ]; then
                break
            fi
            curl -s -X POST localhost:8080 -H 'Content-Type: application/json' \
                --data-binary '{"name":"'${name}${i}'","async":"false","arguments":"{}"}'
        done
    fi
}

function warmup {
    duration=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.warmup_time')
    output="/dev/null"

    warmup_classify
    
    is_warmup=true
    if [ "$approach" = "knative" ]; then
        run_knative
    elif [ "$approach" = "wsk" ]; then
        run_wsk
    elif [ "$approach" = "hydra-si" ]; then
        run_hydra_si
    else
        run_graalvisor
    fi
}

function benchmark {
    duration=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.benchmark_time')
    output="$EXPERIMENT_HOME/$benchmark/$approach/debug/$workload-wrk_output.txt"

    is_warmup=false
    if [ "$approach" = "knative" ]; then
        run_knative
    elif [ "$approach" = "wsk" ]; then
        run_wsk
    elif [ "$approach" = "hydra-si" ]; then
        run_hydra_si
    else
        run_graalvisor
    fi
}

function get_memory {
    OFILE_RSS=$EXPERIMENT_HOME/$benchmark/$approach/memory/$workload-footprint.log
    rm $OFILE_RSS &> /dev/null
    docker container stats --no-stream $(docker container ps -a | grep box | awk '{print $1}') > $OFILE_RSS
}

function run {
    workload_sizes=(1 8 14 20 32)
    if [ "$benchmark" = "gv_classify" ] || [ "$benchmark" = "gv_thumbnail" ]; then
        workload_sizes=(1 8 14)
    fi
    for workload in ${workload_sizes[@]}
    do
        echo "Running $workload concurrent requests"

        if [ "$approach" = "wsk" ] || [ "$approach" = "hydra-si" ]; then
            start_multiple_containers
        else
            start_single_container
        fi

        register_lambda

        warmup

        sleep 10

        benchmark

        get_memory

        stop_containers

        sleep 2
    done
}

function setup {
    mkdir -p $EXPERIMENT_HOME/$benchmark/$approach/{debug,memory,latency}
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

if [ -z "${HYDRA_BENCHMARKS_HOME}" ]; then
    echo "HYDRA_BENCHMARKS_HOME is not defined. Exiting..."
    exit 1
fi

if [ -z "${WEBSERVER_IP}" ]; then
    echo "WEBSERVER_IP is not defined. Exiting..."
    exit 1
fi

trap 'cleanup_resources' SIGINT

for benchmark in gv_bfs gv_classify gv_compression gv_dna gv_dynamic_html gv_mst gv_pagerank gv_thumbnail gv_uploader
do
    approaches=(isolate faastion_lpi faastlane process hydra-si)
    if [ "$benchmark" = "gv_classify" ]; then
        approaches=(faastion_lpi hydra-si)
    fi
    for approach in ${approaches[@]}
    do
        echo -e "${GREEN}## Measuring metrics for $approach - $benchmark ##${NC}"
        setup
        run
    done
done
