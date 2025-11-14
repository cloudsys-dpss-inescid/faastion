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

function get_99p {
    file=$baseline/99p.txt
    for c in $(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    do
        lat=$(cat $baseline/logs/$c-ab*.log | grep '99%' | awk '{sum += $2} END {if (NR > 0) print sum / NR}')
        echo $lat >> $file
    done
}

function get_lat {
    file=$baseline/lat.txt
    for c in $(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    do
        lat=$(cat $baseline/logs/$c-ab*.log | grep 'Total:' | awk '{sum += $3} END {if (NR > 0) print sum / NR}')
        echo $lat >> $file
    done
}

# process `free` command output and create file: mem.txt
function get_memory {
    local mem_arr m1=0
    mem_file=$baseline/mem.txt
    for c in $(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    do
        idle_mem=$(head -n 20 $baseline/logs/$c-mem.log | awk '{sum += $1} END {printf "%.2f\n", sum / 20}')
        max_mem=$(cat $baseline/logs/$c-mem.log | sort -n | tail -n 1)
        mem=$(echo "scale=2; $max_mem - $idle_mem" | bc)
        mem_arr+=($mem)
    done

    # approximate memory of 1st invocation
    threads=$(ls $baseline/logs/ | awk -F- '{print $1}' | sort -u -n)
    threads_arr=($threads)
    for idx in ${!threads_arr[@]}
    do
        aux=$(echo "scale=2; $m1 + (${mem_arr[$idx]} / ${threads_arr[$idx]})" | bc)
        m1=$aux
    done
    
    if [ ${threads_arr[$idx]} -eq 1 ]; then
        mem_arr[0]=$(echo "scale=2; $m1 / ${#threads_arr[@]}" | bc)
    fi

    for mem in ${mem_arr[@]}
    do
        echo "$mem" >> $mem_file
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

function preprocess_benchmark {
    cd $EXPERIMENTS_DIR/$benchmark
    for baseline in $(ls)
    do
        rm -f $baseline/{tput.txt,mem.txt,cpu.txt,99p.txt,lat.txt}
        get_tput
        get_memory
        get_cpu
        get_99p
        get_lat
    done
    cd - &> /dev/null
}

if [ $# -lt 1 ]; then
    echo "Usage: ./preprocess_data.sh <experiments_dir>"
    exit 1
fi

for benchmark in $(ls $EXPERIMENTS_DIR)
do
    preprocess_benchmark
done