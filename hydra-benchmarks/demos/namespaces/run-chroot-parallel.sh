#!/bin/bash

# Cache sudo access.
sudo -l &> /dev/null

rm -r results-chroot/ &> /dev/null
mkdir results-chroot/

for c in 1 2 4 8
do
	for iter in $(seq 5)
	do
		for i in $(seq $c)
		do
			echo "Running chroot $i from $c, iteration $iter..."
			sudo ./run-chroot.sh "/tmp/experiment-$c-$i" >> results-chroot/experiment-$c-$i.log & # TODO - fixme!
			echo "Running chroot $i from $c, iteration $iter... done!"
		done
		wait
	done
	echo $c >> results-chroot/experiment-procs.dat
	cat results-chroot/experiment-$c-* | awk '{print $3}' | ~/git/helper-scripts/math/average.py >> results-chroot/experiment-mean.dat
	cat results-chroot/experiment-$c-* | awk '{print $3}' | ~/git/helper-scripts/math/stdev.py   >> results-chroot/experiment-std.dat
done 
