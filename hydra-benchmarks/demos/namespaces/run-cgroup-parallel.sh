#!/bin/bash

# Cache sudo access.
sudo -l &> /dev/null

rm -r results-cgroup/ &> /dev/null
mkdir results-cgroup/

for c in 1 2 4 8
do
	for iter in $(seq 5)
	do
		for i in $(seq $c)
		do
			echo "Running cgroup $i from $c, iteration $iter..."
			sudo ./run-cgroup.sh "experiment-$c-$i" >> results-cgroup/experiment-$c-$i.log &
			echo "Running cgroup $i from $c, iteration $iter... done!"
		done
		wait
	done
	echo $c >> results-cgroup/experiment-procs.dat
	cat results-cgroup/experiment-$c-* | grep creation | awk '{print $4}' | ~/git/helper-scripts/math/average.py >> results-cgroup/experiment-creation-mean.dat
	cat results-cgroup/experiment-$c-* | grep creation | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py   >> results-cgroup/experiment-creation-std.dat
	cat results-cgroup/experiment-$c-* | grep setup    | awk '{print $4}' | ~/git/helper-scripts/math/average.py >> results-cgroup/experiment-setup-mean.dat
	cat results-cgroup/experiment-$c-* | grep setup    | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py   >> results-cgroup/experiment-setup-std.dat
	cat results-cgroup/experiment-$c-* | grep Adding   | awk '{print $6}' | ~/git/helper-scripts/math/average.py >> results-cgroup/experiment-adding-mean.dat
	cat results-cgroup/experiment-$c-* | grep Adding   | awk '{print $6}' | ~/git/helper-scripts/math/stdev.py   >> results-cgroup/experiment-adding-std.dat
	cat results-cgroup/experiment-$c-* | grep Removing | awk '{print $4}' | ~/git/helper-scripts/math/average.py >> results-cgroup/experiment-removing-mean.dat
	cat results-cgroup/experiment-$c-* | grep Removing | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py   >> results-cgroup/experiment-removing-std.dat
done 
