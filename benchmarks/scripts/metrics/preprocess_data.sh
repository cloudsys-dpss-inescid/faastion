#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)
EXPERIMENTS_DIR=$1

# process ab output and create file: tput.txt 
function get_tput {
    tput_file=$baseline/tput.txt
    for c in $(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    do
        tput=$(cat $baseline/logs/$c-ab*.log | grep 'Requests per second:' | awk '{sum += $4} END {print sum}')
        echo $tput >> $tput_file
    done
}

# process `free` command output and create file: mem.txt 
function get_memory {
    mem_file=$baseline/mem.txt
    for c in $(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    do
        max_mem=$(cat $baseline/logs/$c-mem.log | sort -n | tail -n 1)
        mem=$(echo "scale=2; $max_mem - $IDLE_MEM" | bc)
        echo $mem >> $mem_file
    done
}

function get_cpu_hist {
    cat $baseline/logs/$c-cpu_util.log | \
    awk 'NR > 20 {sum = 0; for (i=2; i <= NF; i++) sum += $i;
        delta = sum - prev_sum; idle = $5 - prev_idle;
        used = delta - idle; print 100 * used / delta}
        {prev_idle = $5; prev_sum = 0; for (i=2; i <= NF; i++) prev_sum += $i}'
}

# process /proc/stat for cpu utilization and create file: cpu.txt
function get_cpu {
    cpu_file=$baseline/cpu.txt
    for c in $(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    do
        hist=$(get_cpu_hist)
        cpu=$(echo "$hist" | awk '{sum += $1} END {if (NR > 0) print sum / NR}')
        echo $cpu >> $cpu_file
    done
}

function get_idle_mem {
    for baseline in $(ls)
    do
        nr=$(($nr + $(ls $baseline/logs/ | grep mem.log | wc -l)))
        idle_mem=$(($idle_mem + $(head -n 20 $baseline/logs/*-mem.log | awk '{sum += $1} END {print sum}')))
    done
    IDLE_MEM=$(echo "scale=2; $idle_mem / (20 * $nr)" | bc)
}

function preprocess_benchmark {
    cd $EXPERIMENTS_DIR/$benchmark
    get_idle_mem
    for baseline in $(ls)
    do
        rm -f $baseline/{tput.txt,mem.txt,cpu.txt}
        get_tput
        get_memory
        get_cpu
    done
    cd - &> /dev/null
}

for benchmark in $(ls $EXPERIMENTS_DIR)
do
    preprocess_benchmark
done