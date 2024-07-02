#!/bin/bash

tests=(
    free_one_chunk
    free_all_chunks
)

# Allow user to invoke the script from anywhere
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

source ../compile_run.sh test_free_fragmented $@
