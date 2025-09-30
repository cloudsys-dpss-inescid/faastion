#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

HOST=localhost
PORT=8080
NAME=pagerank
ENTRYPOINT=com.jni.PageRank

function println {
	echo -n -e '\n'
}

function create_function {
	for i in $(seq 1 $num)
	do
		curl -s -X POST $HOST:$PORT/register?name=${NAME}${i}\&entryPoint=$ENTRYPOINT\&language=java\&sandbox=$sandbox \
		        -H 'Content-Type: application/json' --data-binary @"${DIR}/build/${lib_name}" \
		&& println
	done
}

function run_function {
	for i in $(seq 1 $num)
	do
		curl -s -X POST $HOST:$PORT -H 'Content-Type: application/json' \
		        --data-binary '{"name":"'${NAME}${i}'","async":"false","arguments":"{}"}' \
		&& println
	done
}

sandbox="isolate"

while getopts ":n:PrxX" option; do
	case $option in
	n) # number of functions -- default is 1
		num=$OPTARG;;
	r) # run function(s)
		run_function="true";;
	x) # register faastion function(s)
		create_function="transformed";;
	X) # register vanilla function(s)
		create_function="vanilla";;
	P)
		sandbox="process"
		create_function="vanilla";;
	\?) # Invalid option
		echo "Error: Invalid option"
         	exit;;
	esac
done

if [ ! -z "$num" ]; then
	if [ "$create_function" = "transformed" ]; then
		lib_name="lib${NAME}.so"
		create_function
	elif [ "$create_function" = "vanilla" ]; then
		lib_name="lib${NAME}_vanilla.so"
		create_function
	fi

	if [ "run_function" = "true" ]; then
		run_function
	fi
else
	if [ "$create_function" = "transformed" ]; then
		curl -s -X POST $HOST:$PORT/register?name=${NAME}\&entryPoint=$ENTRYPOINT\&language=java\&sandbox=$sandbox \
                        -H 'Content-Type: application/json' --data-binary @"${DIR}/build/lib${NAME}.so" \
		&& println
	elif [ "$create_function" = "vanilla" ]; then
		curl -s -X POST $HOST:$PORT/register?name=${NAME}\&entryPoint=$ENTRYPOINT\&language=java\&sandbox=$sandbox \
                        -H 'Content-Type: application/json' --data-binary @"${DIR}/build/lib${NAME}_vanilla.so" \
		&& println
	fi

	if [ "run_function" = "true" ]; then
		curl -s -X POST $HOST:$PORT -H 'Content-Type: application/json' \
                        --data-binary '{"name":"'${NAME}'","async":"false","arguments":"{}"}' \
		&& println
        fi
fi
