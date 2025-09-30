#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)
EXPERIMENTS_DIR=$1

# process wrk output and create files: 90p.txt, 50p.txt 
function get_latency {
    p90_file=$baseline/latency/90p.txt
    p50_file=$baseline/latency/50p.txt
    for i in $(ls $baseline/debug/ | awk -F- '{print $1}' | sort -u -n)
    do
        if [ "$baseline" = "wsk" ] || [ "$baseline" = "hydra-si" ]; then
            wrk_file=$baseline/debug/$i-wrk_output-*.txt
        else
            wrk_file=$baseline/debug/$i-wrk_output.txt
        fi
        timestamps=$(cat $wrk_file | grep -o 'process_time(us)"\?:[0-9]*')
        requests=$(echo "$timestamps" | wc -l)
        p90_line=$(echo "($requests * 0.9) / 1" | bc)
        p50_line=$(echo "($requests * 0.5) / 1" | bc)
        p90_latency=$(echo "$timestamps" | awk -F: '{print $2}' | sort -n | awk 'NR=='$p90_line' {print $0}')
        p50_latency=$(echo "$timestamps" | awk -F: '{print $2}' | sort -n | awk 'NR=='$p50_line' {print $0}')
        echo $p90_latency >> $p90_file
        echo $p50_latency >> $p50_file
    done
}

# process wrk output and create file: tput.txt 
function get_tput {
    tput_file=$baseline/latency/tput.txt
    for i in $(ls $baseline/debug/ | awk -F- '{print $1}' | sort -u -n)
    do
        if [ "$baseline" = "wsk" ] || [ "$baseline" = "hydra-si" ]; then
            wrk_file=$baseline/debug/$i-wrk_output-*.txt
        else
            wrk_file=$baseline/debug/$i-wrk_output.txt
        fi
        tput=$(cat $wrk_file | grep 'Requests/sec' | awk '{sum += $2} END {print sum}')
        echo $tput >> $tput_file
    done
}

# process `free` command output and create file: max_footprint.txt 
# function get_memory {
#     memory_file=$baseline/memory/max_footprint.txt
#     for i in $(ls $baseline/memory/ | awk -F- '{print $1}' | sort -n)
#     do
#         mem_log=$baseline/memory/$i-footprint.log
#         mem=$(cat $mem_log | sort -n | awk 'END {print $0}')
#         echo $mem >> $memory_file
#     done
# }

# process docker stats footprint output and create file: max_footprint.txt 
function get_memory {
    memory_file=$baseline/memory/max_footprint.txt
    for i in $(ls $baseline/memory/ | awk -F- '{print $1}' | sort -n)
    do
        mem_log=$baseline/memory/$i-footprint.log
        mem=$(cat $mem_log | awk 'NR>=2 {sum += substr($4, 1, length($4)-3)} END {print sum}')
        echo $mem >> $memory_file
    done
}

function preprocess_benchmark {
    cd $EXPERIMENTS_DIR/$benchmark
    for baseline in $(ls)
    do
        rm -f $baseline/latency/{90p.txt,50p.txt,tput.txt} $baseline/memory/max_footprint.txt
        get_latency
        get_tput
        get_memory
    done
    cd - &> /dev/null
}

for benchmark in $(ls $EXPERIMENTS_DIR)
do
    preprocess_benchmark
done