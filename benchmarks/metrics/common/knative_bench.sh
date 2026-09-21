#!/bin/bash

function knative_registration {
    local benchmark=$1

    entrypoint=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.entrypoint')
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')

    curl -s -X POST "127.0.0.1:8080/register?"\
"entryPoint=$entrypoint"\
"&language=java"\
"&name=$name"\
"&sandbox=context"\
"&url=http://$WEBSERVER_IP:8000/apps/lib$name.zip" &> /dev/null

}

function launch_knative {
    local benchmark=$1

    cpus=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.cpus')
    memory=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.memory')

    if [ "$LIMIT_RESOURCES" = "true" ]; then
        resources="--cpus=\"$cpus\" --memory=\"{$memory}m\""
    fi

    docker run -d --rm -v $ARGO_HOME/graalvisor/shared:/faastion/graalvisor/shared --network host $resources --name sbox faastion &> /dev/null

    while ! nc -z localhost 8080; do sleep 0.01; done
    sleep 2

    knative_registration $benchmark
}

function benchmark_knative {
    local benchmark=$1
    local log_dir=$2
    local c=$3
    ab_log=$log_dir/$c-ab.log

    n=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.req')
    req=$(echo "$n * $c" | bc)
    n=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.warmup_req')
    warmup_req=$(echo "$n * $c" | bc)
    name=$(jq -n --arg webserver "$WEBSERVER_IP" -f $DIR/data.json | jq -r '.'$benchmark'.lib_name')

    jq -n --arg name $name -f $DIR/template.json > $DIR/post.json

    # warmup
    ab -l -p $DIR/post.json -T application/json -c $c -n $warmup_req localhost:8080/ &> /dev/null

    # collect results
    ab -l -p $DIR/post.json -T application/json -c $c -n $req localhost:8080/ &> $ab_log

    # validate response content
    response=$(curl -s -X POST localhost:8080 -H 'Content-Type: application/json' --data-binary '{"name":"'$name'","async":"false","arguments":"{}"}')
    echo $response >> $ab_log
}

function tput_knative {
    local benchmark=$1 log_dir=$2 c=$3
    tput=$(cat $log_dir/$c-ab.log | grep 'Requests per second:' | awk '{print $4}')
    echo $tput
}
