#!/bin/bash

# Cache sudo access.
sudo -l &> /dev/null

rm -r results-netns/ &> /dev/null
mkdir results-netns/

for c in 1 2 4 8
do
	for iter in $(seq 5)
	do
		for i in $(seq $c)
		do
			echo "Running netns $i from $c, iteration $iter..."
			sudo ./run-netns.sh $i &>> results-netns/experiment-$c-$i.log &
			echo "Running netns $i from $c, iteration $iter... done!"
		done
		wait
		sleep 1 # This sleep is necessary because some resources take time to be deleted eventhough we wait for all subprocesses.
	done
	echo $c >> results-netns/experiment-procs.dat
	cat results-netns/experiment-$c-* | grep "Netns creation" | awk '{print $4}' | ~/git/helper-scripts/math/average.py  >> results-netns/experiment-netnscreation-mean.dat
	cat results-netns/experiment-$c-* | grep "Netns creation" | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py    >> results-netns/experiment-netnscreation-std.dat
	cat results-netns/experiment-$c-* | grep "Tap creation"   | awk '{print $4}' | ~/git/helper-scripts/math/average.py  >> results-netns/experiment-tapcreation-mean.dat
	cat results-netns/experiment-$c-* | grep "Tap creation"   | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py    >> results-netns/experiment-tapcreation-std.dat
	cat results-netns/experiment-$c-* | grep "Veth creation"  | awk '{print $4}' | ~/git/helper-scripts/math/average.py  >> results-netns/experiment-vethcreation-mean.dat
	cat results-netns/experiment-$c-* | grep "Veth creation"  | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py    >> results-netns/experiment-vethcreation-std.dat
	cat results-netns/experiment-$c-* | grep "Veth setup"     | awk '{print $4}' | ~/git/helper-scripts/math/average.py  >> results-netns/experiment-vethsetup-mean.dat
	cat results-netns/experiment-$c-* | grep "Veth setup"     | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py    >> results-netns/experiment-vethsetup-std.dat
	cat results-netns/experiment-$c-* | grep "Routes setup"   | awk '{print $4}' | ~/git/helper-scripts/math/average.py  >> results-netns/experiment-routessetup-mean.dat
	cat results-netns/experiment-$c-* | grep "Routes setup"   | awk '{print $4}' | ~/git/helper-scripts/math/stdev.py    >> results-netns/experiment-routessetup-std.dat
	cat results-netns/experiment-$c-* | grep "Deletion took"  | awk '{print $3}' | ~/git/helper-scripts/math/average.py  >> results-netns/experiment-deletion-mean.dat
	cat results-netns/experiment-$c-* | grep "Deletion took"  | awk '{print $3}' | ~/git/helper-scripts/math/stdev.py    >> results-netns/experiment-deletion-std.dat
done 
