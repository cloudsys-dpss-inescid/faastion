#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)

function usage {
    echo "Usage: benchmark.sh <option>"
    echo "Options:"
    echo "  1. dna"
    echo "  2. dynamic-html"
    exit
}

if [ $# -lt 1 ]; then
    usage
elif [ $1 -eq 1 ]; then
    NAME=dna
    CLASSNAME=com.dna.DNAVisualization
elif [ $1 -eq 2 ]; then
    NAME=dynamic-html
    CLASSNAME=com.dynamic_html.DynamicHTML
else
    usage
fi

BIN=$ARGO_HOME/core/build/native-image/polyglot-proxy  
RESULTS_DIR="results/$NAME"
ISOLATE_CSV=$RESULTS_DIR/isolate_sandbox.csv
CONTEXT_CSV=$RESULTS_DIR/context_sandbox.csv

library_path="$HOME/usr/lib:$ARGO_HOME/core/shared:$LIBC_HOME/lib:/lib/x86_64-linux-gnu"

function hydra_registration {
    curl -s -X POST "127.0.0.1:8080/register?"\
"entryPoint=$CLASSNAME"\
"&language=java"\
"&name=$NAME"\
"&sandbox=$SANDBOX"\
"&url=http://$WEBSERVER_IP:8000/apps/lib$NAME.zip" &> /dev/null
}

function run {
    local gc_time_us reqs total_gc_time lat

    ITER=4
    for try in $(seq 1 $ITER)
    do
        log_gc=$GC_LOG LD_LIBRARY_PATH=$library_path $BIN &> "output" &
        PID=$!
        while ! nc -z localhost 8080; do sleep 0.01; done
        sleep 0.2
        hydra_registration
        env function_name=$NAME wrk -H "Connection: Close" --latency -t$concurrency -c$concurrency -d5s -T5s -s native.lua http://127.0.0.1:8080 &> wrk.out
        kill -9 $PID
        wait $PID &> /dev/null
        # gc_time_us+=$(cat "output" | grep -o 'time: [0-9]*' | awk '{sum += $2} END {if (NR > 0) print sum / (NR * 1000)}')
        # lat+=$(cat "wrk.out" | grep -o 'time (us)":[0-9]*' | awk -F: '{sum += $2} END {if (NR > 0) printf "%.2f\n", sum / NR}')
        reqs+=$(cat "wrk.out" | grep 'Requests/sec' | awk '{print $2}')
        total_gc_time+=$(cat "output" | grep -o 'time: [0-9]*' | awk '{sum += $2} END {print sum}')
        if [ $try -ne $ITER ]; then
            # gc_time_us+=+
            # lat+=+
            reqs+=+
            total_gc_time+=+
        fi
    done

    # avg_gc_time=$(echo "scale=2; ($total_gc_time) / $ITER" | bc)
    # avg_lat=$(echo "scale=2; ($lat) / $ITER" | bc)
    avg_reqs=$(echo "scale=2; ($reqs) / $ITER" | bc)
    avg_gc_total=$(echo "scale=2; ($total_gc_time) / $ITER" | bc)
    echo "$concurrency,$avg_gc_total,$avg_reqs" >> $OUTFILE
}

function run_isolate_sandbox {
    SANDBOX=isolate
    OUTFILE=$ISOLATE_CSV
    rm -f $OUTFILE
    for concurrency in 1 2 5 10 15 20
    do
        echo "Running isolate sandbox - $concurrency parallel threads"
        for ix in $(seq 1 $concurrency)
        do
            GC_LOG=$ix
            run
        done
    done
}

function run_context_sandbox {
    SANDBOX=context
    OUTFILE=$CONTEXT_CSV
    rm -f $OUTFILE
    for concurrency in 1 2 5 10 15 20
    do
        echo "Running context sandbox - $concurrency parallel threads"
        GC_LOG=1
        run
    done
}

cd $DIR
mkdir -p $RESULTS_DIR

run_isolate_sandbox
# run_context_sandbox

cd - &> /dev/null