#!/bin/bash

tests=(
    memcpy_test
)

# Allow user to invoke the script from anywhere
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

source ../compile_run.sh test_memcpy $@
