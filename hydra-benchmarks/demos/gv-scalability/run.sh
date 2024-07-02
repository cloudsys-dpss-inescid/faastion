#!/bin/bash

rm -r results &> /dev/null
mkdir results &> /dev/null
rm *.dat      &> /dev/null
for mode in process thread
do
	for isolates in 1 2 4 8 16 32 64 128
	do
		for attempt in 1 2 3 4 5
		do
			echo "Running $mode with $isolates attempt $attempt..."
			build/graalvisorhost $mode true $isolates ../gv-guest/build/graalvisorguest.so GraalvisorGuestIsolateBenchmark &> results/run-$mode-$isolates-$attempt.log
			echo "Running $mode with $isolates attempt $attempt... done!"
			if ! grep -q SubstrateSegfaultHandler "results/run-$mode-$isolates-$attempt.log"; then
				cat results/run-$mode-$isolates-$attempt.log | grep -v "Memory" >> results/latency-$mode-$isolates.dat
				cat results/run-$mode-$isolates-$attempt.log | grep "Memory" | awk '{print $7}' >> results/rss-$mode-$isolates.dat
				cat results/run-$mode-$isolates-$attempt.log | grep "Memory" | awk '{print $9}' >> results/pss-$mode-$isolates.dat
			fi
			sleep 1
		done
	done
done
