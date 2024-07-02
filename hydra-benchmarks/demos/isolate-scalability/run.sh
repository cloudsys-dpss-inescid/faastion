#!/bin/bash

rm -r results &> /dev/null
mkdir results &> /dev/null
for isolates in 1 2 4 8 16 32 64 128 256 512 1024
do
	for attempt in 1 2 3 4 5
	do
		echo "Running with $isolates attempt $attempt..."
		/bin/time -v build/isolate-scalability $isolates &> results/run-$isolates-$attempt.log
		echo "Running with $isolates attempt $attempt... done!"
		sleep 1
	done
	cat results/run-$isolates-*.log | grep took | awk '{print $5}' >> results/latency-$isolates.dat
	cat results/run-$isolates-*.log | grep "Maximum resident set size" | awk '{print $6}' >> results/memory-$isolates.dat
done
