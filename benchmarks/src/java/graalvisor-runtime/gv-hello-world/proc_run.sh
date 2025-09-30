#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

HOST=localhost
PORT=8080
#NAME=nativehw1

NAME=$1

function println {
	echo -n -e '\n'
}

curl -s -X POST $HOST:$PORT/register?name=$NAME\&entryPoint=com.hello_world.HelloWorld\&language=java\&sandbox=process \
	-H 'Content-Type: application/json' --data-binary @"${DIR}/build/lib${NAME}.so" \
&& println \
&& curl -s -X POST $HOST:$PORT -H 'Content-Type: application/json' \
	--data-binary '{"name":"'${NAME}'","async":"false","arguments":"{}"}' \
&& println
