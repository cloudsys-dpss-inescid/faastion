#!/bin/bash

DIR=$(cd $(dirname $0) && pwd)
source $DIR/../utils/shared.sh
BIN=$DIR/../build/bin/test_seccomp
MODE="$BIN --mode-strict"
compile_seccomp
run $@
