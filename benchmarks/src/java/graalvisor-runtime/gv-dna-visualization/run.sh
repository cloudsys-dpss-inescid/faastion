#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

HOST=localhost
PORT=8080
#NAME=dna

NAME=$1

function println {
	echo -n -e '\n'
}

curl -s -X POST $HOST:$PORT/register?name=$NAME\&entryPoint=com.dna.DNAVisualization\&language=java \
	-H 'Content-Type: application/json' --data-binary @"${DIR}/build/lib${NAME}.so" \
&& println \
&& curl -s -X POST $HOST:$PORT -H 'Content-Type: application/json' \
	--data-binary '{"name":"'${NAME}'","async":"false","arguments":"{}"}' \
&& println
