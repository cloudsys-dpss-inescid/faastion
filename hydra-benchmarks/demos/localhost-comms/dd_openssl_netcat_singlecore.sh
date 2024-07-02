#!/bin/bash

set -e;

function run_nc () 
{
    taskset -c 0 dd if=/dev/zero bs=$1 count=$2 | taskset -c 0 nc -c localhost 12345
}

function run_openssl_nc () 
{
    taskset -c 0 dd if=/dev/zero bs=$1 count=$2 | taskset -c 0 openssl aes-256-cbc -pass pass:test | taskset -c 0 nc -c localhost 12345
}

block_sizes=(
    32
    64
    128
    256
    512 
    1024
    $((16*1024))
    $((128*1024))
    $((512*1024))
    $((1024*1024))
    $((8*1024*1024))
)

for block_size in ${block_sizes[@]}; do 
    count=$((2**30 / $block_size))

    echo "dd + nc: $block_size $count"
    nc -l -p 12345 > /dev/null &
    t1=$(date +%s%3N)
    run_nc $block_size $count
    t2=$(date +%s%3N)
    t_nc=$(($t2-$t1))
    echo "total: $t_nc ms"
    
    echo "dd + openssl + nc: $block_size $count"
    nc -l -p 12345 > /dev/null &
    t1=$(date +%s%3N)
    run_openssl_nc $block_size $count
    t2=$(date +%s%3N)
    t_openssl_nc=$(($t2-$t1))
    echo "total: $t_openssl_nc ms"

    echo "--------------------------------------"
    echo "| $block_size B | $t_nc ms | $t_openssl_nc ms |"
    echo "--------------------------------------"
done


