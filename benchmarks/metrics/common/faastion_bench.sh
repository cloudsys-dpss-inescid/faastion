#!/bin/bash

function faastion_registration {
    local benchmark=$1
    local c=$2

    entrypoint=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.entrypoint')
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')

    if [ "$benchmark" = "gv_classify" ]; then
        faastion_register_classify $entrypoint $name $c
    else
    curl -s -X POST "127.0.0.1:8080/register?"\
"entryPoint=$entrypoint"\
"&language=java"\
"&name=$name"\
"&sandbox=pku"\
"&url=http://$WEBSERVER_IP:8000/apps/lib${name}-plugin.zip" &> /dev/null
    fi

}

function launch_faastion {
    local benchmark=$1
    local c=$2

    cpus=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.cpus')
    memory=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.memory')
    active_wait=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.active_wait')

    if [ "$active_wait" != "null" ]; then
        active_wait_env="-e ACTIVE_WAIT_CAP=$active_wait"
    fi

    if [ "$LIMIT_RESOURCES" = "true" ]; then
        resources="--cpus=\"$cpus\" --memory=\"{$memory}m\""
    fi

    docker run -d --rm -v $ARGO_HOME/graalvisor/shared:/faastion/graalvisor/shared --network host $active_wait_env $resources --name sbox faastion --enable-lpi &> /dev/null

    while ! nc -z localhost 8080; do sleep 0.01; done
    sleep 2

    faastion_registration $benchmark $c
}

function log_subprocesses {
    local logfile=$1

    rm -f $logfile
    sleep 1
    while nc -z localhost 8080
    do
        docker exec sbox ps aux | grep -E -v 'ps aux|start.sh' | wc -l | xargs printf "%d - 2\n" | bc &>> $logfile
        sleep 1
    done
}

function benchmark_faastion {
    local benchmark=$1
    local log_dir=$2
    local c=$3

    if [ "$benchmark" = "gv_classify" ]; then
        faastion_benchmark_classify $benchmark $log_dir $c
        return
    fi

    proc_log=$log_dir/$c-proc_count.log
    ab_log=$log_dir/$c-ab.log

    log_subprocesses $proc_log &

    n=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.req')
    req=$(echo "$n * $c" | bc)
    n=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.warmup_req')
    warmup_req=$(echo "$n * $c" | bc)
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')

    mkdir -p /tmp/faastion
    JSON_FILE=/tmp/faastion/post.json

    jq -n --arg name $name -f $COMMON/template.json > $JSON_FILE

    # warmup
    ab -l -p $JSON_FILE -T application/json -c $c -n $warmup_req localhost:8080/ &> /dev/null

    # collect results
    ab -l -p $JSON_FILE -T application/json -c $c -n $req localhost:8080/ &> $ab_log

    # validate response content
    response=$(curl -s -X POST localhost:8080 -H 'Content-Type: application/json' --data-binary '{"name":"'$name'","async":"false","arguments":"{}"}')
    echo $response >> $ab_log
}

function tput_faastion {
    local benchmark=$1 log_dir=$2 c=$3

    if [ "$benchmark" = "gv_classify" ]; then
	tput=$(faastion_tput_classify $log_dir $c)
    else
	tput=$(cat $log_dir/$c-ab.log | grep 'Requests per second:' | awk '{print $4}')
    fi
    echo $tput
}

##############################################################################
###         Temporary workaround to handle classify in faastion            ###
##############################################################################

function faastion_warmup_classify {
    local warmup_req=$1 c=$2
    for i in $(seq 1 $c)
    do
        JSON_FILE=/tmp/faastion/post-$i.json
        ab -l -p $JSON_FILE -T application/json -c 1 -n $warmup_req localhost:8080/ &> /dev/null &
    done
    wait
}

function faastion_collect_classify_results {
    local log_dir=$1 req=$2 c=$3
    for i in $(seq 1 $c)
    do
        ab_log=$log_dir/$c-ab-$i.log
        JSON_FILE=/tmp/faastion/post-$i.json
        ab -l -p $JSON_FILE -T application/json -c 1 -n $req localhost:8080/ &> $ab_log &
    done
    wait
}

function faastion_validate_classify {
    local log_dir=$1 name=$2 c=$3
    for i in $(seq 1 $c)
    do
        ab_log=$log_dir/$c-ab-$i.log
        curl -s -X POST localhost:8080 -H 'Content-Type: application/json' --data-binary '{"name":"'${name}${i}'","async":"false","arguments":"{}"}' >> $ab_log &
    done
    wait
}

function faastion_benchmark_classify {
    local benchmark=$1
    local log_dir=$2
    local c=$3

    req=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.req')
    warmup_req=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.warmup_req')
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')

    mkdir -p /tmp/faastion

    for i in $(seq 1 $c)
    do
        JSON_FILE=/tmp/faastion/post-$i.json
        jq -n --arg name ${name}${i} -f $COMMON/template.json > $JSON_FILE
    done

    # make sure that each sandbox initializes tensorflow
    for i in $(seq 1 $c)
    do
        curl -s -X POST localhost:8080 -H 'Content-Type: application/json' --data-binary '{"name":"'${name}${i}'","async":"false","arguments":"{}"}' &> /dev/null
    done

    faastion_warmup_classify $warmup_req $c &
    local pid=$!
    wait $pid

    faastion_collect_classify_results $log_dir $req $c &
    local pid=$!
    wait $pid

    faastion_validate_classify $log_dir $name $c &
    local pid=$!
    wait $pid
}

function faastion_register_classify {
    local entrypoint=$1 name=$2 c=$3

    for i in $(seq 1 $c)
    do
      curl -s -X POST "127.0.0.1:8080/register?"\
"entryPoint=$entrypoint"\
"&language=java"\
"&name=${name}${i}"\
"&sandbox=pku"\
"&url=http://$WEBSERVER_IP:8000/apps/lib${name}-plugin.zip" &> /dev/null
    done

}

function faastion_tput_classify {
    local log_dir=$1 c=$2
    tput=$(cat $log_dir/$c-ab*.log | grep 'Requests per second:' | awk '{sum += $4} END {if (NR == '$c') print sum}')
    echo $tput
}
