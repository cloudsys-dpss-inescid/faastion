#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}


# We are importing the single function shared...
source $(DIR)/../cr-single-function/shared.sh

# ... and overwriting some of its definitions.
BENCHMARK_RUN_ENDPOINT[jshw]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[jsdh]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[jsup]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[jstn]="" # Note: cr not supported.

BENCHMARK_RUN_ENDPOINT[pyhw]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pymst]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pybfs]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pypr]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pydna]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pydh]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pyco]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pytn]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pyup]="warmup?concurrency=2&requests=1"
BENCHMARK_RUN_ENDPOINT[pyvp]="" # Note: cr not supported.
