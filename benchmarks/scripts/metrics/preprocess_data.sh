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
        mem=$(cat $baseline/logs/$c-mem*.log | sort -n | tail -n 1)
        echo $mem >> $mem_file
    done
}

# process /proc/stat for cpu utilization and create file: cpu.txt
function get_cpu {
    cpu_file=$baseline/cpu.txt
    for c in $(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    do
        cpu=$(cat $baseline/logs/$c-cpu_util.log | awk 'NR > 1 {printf "%d\n", $2 - prev} {prev = $2}' | awk '{sum += $1} END {if (NR > 0) print sum / NR}')
        echo $cpu >> $cpu_file
    done
}

function preprocess_benchmark {
    cd $EXPERIMENTS_DIR/$benchmark
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