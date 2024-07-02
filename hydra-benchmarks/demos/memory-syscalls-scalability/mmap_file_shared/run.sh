#!/bin/bash

tests=(
    mmap_no_init
    mmap_init
    mmap_no_init_no_unmap
    mmap_init_no_unmap
)

# Allow user to invoke the script from anywhere
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd $DIR

source ../compile_run.sh test_mmap_file_shared $@
