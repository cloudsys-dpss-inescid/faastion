#!/bin/bash

rm *.out
set -xe;

source=$1
gcc $source.c ../utils/*.c -g -o $source.out -Wall -Wextra

shift

for test in "${tests[@]}"; do
    ./$source.out $test $@ > $test.log 2>&1;
    exc=$?
    if [ $exc -ne 0 ]; then
        echo "./$source.out exited with exit code $exc"
    fi
done
