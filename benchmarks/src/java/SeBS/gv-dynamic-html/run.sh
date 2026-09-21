#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

HOST=localhost
PORT=8080
#NAME=dynamic-html

NAME=$1

function println {
	echo -n -e '\n'
}

curl -s -X POST $HOST:$PORT/register?name=$NAME\&entryPoint=com.dynamic_html.DynamicHTML\&language=java \
	-H 'Content-Type: application/json' --data-binary @"${DIR}/build/lib${NAME}.so" \
&& println \
&& curl -s -X POST $HOST:$PORT -H 'Content-Type: application/json' \
	--data-binary '{"name":"'${NAME}'","async":"false","arguments":"{}"}' \
&& println
