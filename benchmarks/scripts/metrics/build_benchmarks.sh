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

function gv_aes_encryption {
        BENCHMARK_DIR="$BENCHMARKS_HOME/src/java/gv-aes-encryption"
}

function gv_hello_world {
	BENCHMARK_DIR="$BENCHMARKS_HOME/src/java/gv-hello-world"
}

if [ -z $CONCURRENCY_LEVEL ]; then
	export CONCURRENCY_LEVEL=32
fi

native_benchmarks=(gv_native_hw gv_native_factors gv_filehashing)
managed_benchmarks=(gv_hello_world gv_aes_encryption)

build_duration=$(echo "scale=2; ${#native_benchmarks[@]} * 40 / 60" | bc)
total_duration=$(echo "scale=2; ($CONCURRENCY_LEVEL + 1) * $build_duration" | bc)
total_duration=$(echo "scale=2; $total_duration + (${#managed_benchmarks[@]} * 40 / 60)" | bc)
echo "Estimated build time ~= $total_duration mins"

for benchmark_name in ${native_benchmarks[@]}
do
	$benchmark_name
	cd "$BENCHMARK_DIR"
	./build_script.sh
	cd -
done

for benchmark_name in ${managed_benchmarks[@]}
do
	$benchmark_name
	cd "$BENCHMARK_DIR"
	./build_script.sh
	cd -
done
