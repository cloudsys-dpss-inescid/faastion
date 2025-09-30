#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

source $(DIR)/../cr-single-function/shared.sh

function run_benchmark {
    # Clean logs but not snapshots.
    bash $(DIR)/cleanup.sh

    # Start hydra.
    start_hydra

    # Upload all functions.
    for bench in "${BENCH_ARRAY[@]}"; do
        upload_function $bench
    done

    # Run ab for each function.
    for bench in "${BENCH_ARRAY[@]}"; do
        run_ab $bench 1 100
    done

    # Run ab for each function.
    for bench in "${BENCH_ARRAY[@]}"; do
        run_ab $bench 1 100
    done

    # Stop hydra.
    stop_hydra
}

run_benchmark
echo "Finished!"
