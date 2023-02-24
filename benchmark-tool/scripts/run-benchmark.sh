#!/bin/bash

OPTION=$1
TO_TEST=$2

# Cache sudo access.
sudo -l &> /dev/null

rm -r results/ &> /dev/null
mkdir results/

for num in 1 2 4 8 16 32 64 128
do
	echo "Processing with ${num} ${TO_TEST}..."
	for i in $(seq 10) 
	do	
		if [ "$TO_TEST" = "threads" ]; then
			./benchmark ${OPTION} $num 1 >> results/${OPTION}-${TO_TEST}-$num-$i.log &
		elif [ "$TO_TEST" = "pages" ]; then
			./benchmark ${OPTION} 1 $num >> results/${OPTION}-${TO_TEST}-$num-$i.log &
		fi
	done
	wait

	echo $num >> results/num.dat
	echo "Calculating average and standard deviation..."
	cat results/${OPTION}-${TO_TEST}-$num-* | scripts/math/mean.py >> results/${OPTION}-${TO_TEST}-mean.dat
	cat results/${OPTION}-${TO_TEST}-$num-* | scripts/math/stdev.py   >> results/${OPTION}-${TO_TEST}-stdev.dat
	echo "Done processing with ${num} ${TO_TEST}."
done 

echo "Generating plot..."
python3 scripts/generate-plot.py --option ${OPTION} --to-test ${TO_TEST}
echo "All done!"