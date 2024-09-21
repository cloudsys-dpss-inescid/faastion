#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

HOST=localhost
PORT=8080
NAME=nativehw

function println {
	echo -n -e '\n'
}

curl -s -X POST $HOST:$PORT/register?name=$NAME\&entryPoint=com.jni.HelloJNI\&language=java \
	-H 'Content-Type: application/json' --data-binary @"${DIR}/build/libnativehw.so" \
&& println \
&& curl -s -X POST $HOST:$PORT -H 'Content-Type: application/json' \
	--data-binary '{"name":"'${NAME}'","async":"false","arguments":"{}"}' \
&& println
