#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
BENCHMARKS_HOME=$DIR/../..

function gv_native_hw {
	BENCHMARK_DIR="$BENCHMARKS_HOME/src/java/gv-native-hw"
}

function gv_native_matmul {
	BENCHMARK_DIR="$BENCHMARKS_HOME/src/java/gv-native-matmul"
}

function gv_native_factors {
	BENCHMARK_DIR="$BENCHMARKS_HOME/src/java/gv-native-factorization"
}

function gv_filehashing {
	BENCHMARK_DIR="$BENCHMARKS_HOME/src/java/gv-file-hashing"
}

if [ -z $CONCURRENCY_LEVEL ]; then
	export CONCURRENCY_LEVEL=32
fi

for benchmark_name in gv_native_hw gv_native_matmul gv_native_factors gv_filehashing
do
	$benchmark_name
	cd "$BENCHMARK_DIR"
	./build_script.sh
	cd -
done
