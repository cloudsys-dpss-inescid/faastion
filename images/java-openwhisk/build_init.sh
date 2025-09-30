#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

gcc -c $DIR/init.c -o $DIR/init.o
gcc -c $DIR/../shared/random.c -o $DIR/random.o
gcc -o $DIR/init $DIR/init.o $DIR/random.o
