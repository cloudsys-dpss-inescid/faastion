#!/bin/bash

OPTION=$1

# Cache sudo access.
sudo -l &> /dev/null

rm -r results/ &> /dev/null
mkdir results/

for num in 1 2 4 8 16 32 64 128 256 512 1024
do
	# Fixed number of pages
	./benchmark ${OPTION} $num 1 >> results/${OPTION}-fixed-pages.dat &

	# Fixed number of threads
	./benchmark ${OPTION} 1 $num >> results/${OPTION}-fixed-threads.dat &
done 

