#!/bin/bash

tests=(
    malloc_no_init
    malloc_init
    malloc_no_init_no_free
    malloc_init_no_free
)

# Allow user to invoke the script from anywhere
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

source ../compile_run.sh test_malloc_fragmented $@
