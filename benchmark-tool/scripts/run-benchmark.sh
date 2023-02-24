#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

OPTION=$1
TO_TEST=$2

# Cache sudo access.
sudo -l &> /dev/null

rm -r $DIR/results/ &> /dev/null
mkdir $DIR/results/

for num in 1 2 4 8 16 32 64 128
do
	echo "Processing with ${num} ${TO_TEST}..."
	for i in $(seq 10)
	do	
		if [ "$TO_TEST" = "threads" ]; then
			$DIR/../benchmark ${OPTION} $num 1 >> $DIR/results/${OPTION}-${TO_TEST}-$num-$i.log &
		elif [ "$TO_TEST" = "pages" ]; then
			$DIR/../benchmark ${OPTION} 1 $num >> $DIR/results/${OPTION}-${TO_TEST}-$num-$i.log &
		fi
	done
	wait

	echo $num >> results/num.dat
	echo "Calculating average and standard deviation..."
	cat $DIR/results/${OPTION}-${TO_TEST}-$num-* | $DIR/../scripts/math/mean.py >> $DIR/results/${OPTION}-${TO_TEST}-mean.dat
	cat $DIR/results/${OPTION}-${TO_TEST}-$num-* | $DIR/../scripts/math/stdev.py   >> $DIR/results/${OPTION}-${TO_TEST}-stdev.dat
	echo "Done processing with ${num} ${TO_TEST}."
done 

echo "Generating plot..."
python3 $DIR/../scripts/generate-plot.py --option ${OPTION} --to-test ${TO_TEST}
echo "All done!"
