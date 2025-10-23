#!/bin/bash

function hydra_si_registration {
    local benchmark=$1
    local c=$2

    entrypoint=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.entrypoint')
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')

    for i in $(seq 1 $c)
    do
        port=$((8080 + $i))
        curl -s -X POST "127.0.0.1:$port/register?"\
"entryPoint=$entrypoint"\
"&language=java"\
"&name=$name"\
"&sandbox=context"\
"&url=http://$WEBSERVER_IP:8000/apps/lib$name.zip" &> /dev/null
    done

}

function launch_hydra_si {
    local benchmark=$1
    local c=$2

    cpus=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.cpus')
    memory=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.memory')

    if [ "$LIMIT_RESOURCES" = "true" ]; then
        resources="--cpus=\"$cpu_count\" --memory=\"{$guest_memory}m\""
    fi

    for i in $(seq 1 $c)
    do
        port=$((8080 + $i))
        docker run -d --rm -v $ARGO_HOME/graalvisor/shared:/faastion/graalvisor/shared -e lambda_port=$port --network host $resources --name sbox-$i faastion &> /dev/null
    done
    
    for i in $(seq 1 $c)
    do
        port=$((8080 + $i))
        while ! nc -z localhost $port; do sleep 0.01; done
    done
    sleep 2

    hydra_si_registration $benchmark $c
}

function warmup_hydra_si {
    local warmup_req=$1 c=$2

    for i in $(seq 1 $c)
    do
        port=$((8080 + $i))
        ab -l -p $DIR/post.json -T application/json -c 1 -n $warmup_req localhost:$port/ &> /dev/null &
    done

    wait
}

function collect_results_hydra_si {
    local log_dir=$1 req=$2 c=$3

    for i in $(seq 1 $c)
    do
        port=$((8080 + $i))
        ab_log=$log_dir/$c-ab-$i.log
        ab -l -p $DIR/post.json -T application/json -c 1 -n $req localhost:$port/ &> $ab_log &
    done

    wait
}

function validate_hydra_si {
    local log_dir=$1 name=$2 c=$3

    for i in $(seq 1 $c)
    do
        port=$((8080 + $i))
        ab_log=$log_dir/$c-ab-$i.log
        curl -s -X POST localhost:$port -H 'Content-Type: application/json' --data-binary '{"name":"'$name'","async":"false","arguments":"{}"}' >> $ab_log &
    done

    wait
}

function benchmark_hydra_si {
    local benchmark=$1
    local log_dir=$2
    local c=$3

    req=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.req')
    warmup_req=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.warmup_req')
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')

    jq -n --arg name $name -f $DIR/template.json > $DIR/post.json

    warmup_hydra_si $warmup_req $c &
    local pid=$!
    wait $pid

    collect_results_hydra_si $log_dir $req $c &
    local pid=$!
    wait $pid

    validate_hydra_si $log_dir $name $c &
    local pid=$!
    wait $pid
}

function tput_hydra_si {
    local benchmark=$1 log_dir=$2 c=$3
    tput=$(cat $log_dir/$c-ab*.log | grep 'Requests per second:' | awk '{sum += $4} END {if (NR == '$c') print sum}')
    echo $tput
}
