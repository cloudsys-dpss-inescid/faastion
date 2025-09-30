#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

source $(DIR)/shared.sh

function prepare_snapshots {
    # Ensure a clean setup (no previous snapshots).
    bash $(DIR)/cleanup-snapshots.sh
    bash $(DIR)/cleanup.sh

    # For each benchmark.
    for bench in "${BENCH_ARRAY[@]}"; do
        # Start hydra.
        start_hydra

        # Upload the function.
        upload_function $bench

        # Create the snapshot after 1 request.
        APP_POST="/tmp/app-post-$bench"
        echo ${BENCHMARK_POST["$bench"]} > $APP_POST
        curl -s -X POST http://$HYDRA_ADDRESS/warmup?requests=1\&concurrency=1 -H "Content-Type: application/json" -d@$APP_POST
        echo ""
        rm $APP_POST

        # Stop hydra.
        stop_hydra

        # Wait for all subprocesses to terminate (hydra in particular).
        wait
    done
}

prepare_snapshots
echo "Finished!"
