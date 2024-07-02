#!/bin/bash

APP=$BENCHMARKS_HOME/src/java/gv-shopcart/target/shopcart-0.3.6.jar
WORKLOADS=$DIR/../workloads
HOST=localhost
PORT=8080

function start_micronaut {
        $MODE java -jar $APP &> /dev/null &
        PID=$!
        while ! nc -z $HOST $PORT; do
                sleep 1
        done
        echo "Running $(basename $DIR)"
}

function stop_micronaut {
        kill -15 $PID
	while lsof -i :8080; do
		sleep .1
	done
}

function warmup {
	ab -n 1000 -T 'application/json' -p $WORKLOADS/create-shop-cart.json $HOST:$PORT/ &> /dev/null
        ab -n 1000 $HOST:$PORT/$(cat $WORKLOADS/cart.id) &> /dev/null
}

function benchmark {
	output=$DIR/post.log
	ab -n $1 -T 'application/json' -p $WORKLOADS/create-shop-cart.json $HOST:$PORT/ > $output
	echo "Check results: $output"
	output=$DIR/get.log
	ab -n $1 $HOST:$PORT/$(cat $WORKLOADS/cart.id) > $output
	echo "Check results: $output"
}

function run {
	start_micronaut
	warmup
	benchmark $1
	stop_micronaut
}

function compile_seccomp {
	cd $DIR/..
	make > /dev/null
	cd - > /dev/null
}

if [ $# -eq 0 ]; then
	echo "usage $0 iter_count"
	exit 1
fi

if [ -z $BENCHMARKS_HOME ]; then
	echo "Please set BENCHMARKS_HOME first"
        exit 1
fi

if [ ! -f $APP ]; then
	echo "Please build shopcart first"
	exit 1
fi
