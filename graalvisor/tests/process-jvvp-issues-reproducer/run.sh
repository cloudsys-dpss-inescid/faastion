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

    # Upload.
    upload_function jvvp

    # Run the function once.
    run_ab jvvp 1 1

    # Run ab.
    run_ab jvvp 5 100

    # Stop hydra.
    stop_hydra
}

run_benchmark
echo "Finished!"
