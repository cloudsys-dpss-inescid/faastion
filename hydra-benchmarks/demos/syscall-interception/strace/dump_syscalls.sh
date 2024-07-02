#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)
source $DIR/../utils/shared.sh 0
MODE="strace -f -c -I 2 -o $DIR/strace.out"
start_micronaut > /dev/null
curl -s -X POST $HOST:$PORT/ -H 'Content-Type: application/json' --data @"$WORKLOADS/create-shop-cart.json" > /dev/null
stop_micronaut > /dev/null
echo "Check dump: $DIR/strace.out"
