#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

OPTION=$1
TO_TEST=$2

# Cache sudo access.
sudo -l &> /dev/null

mkdir $DIR/results &> /dev/null

function domain_vary_threads {
    NUM_PAGES=64000
    rm $DIR/results/domain-threads-* &> /dev/null
    rm $DIR/results/num-threads.dat &> /dev/null
    for num in 1 2 3 4 5 6 7 
    do
        echo "Processing with ${num} threads..."
        for i in $(seq 10)
        do
            $DIR/../benchmark domain $num $NUM_PAGES >> $DIR/results/domain-threads-$num-$NUM_PAGES-$i.log
        done

	echo "Calculating average and standard deviation..."
        cat $DIR/results/domain-threads-$num-$NUM_PAGES-* | $DIR/../scripts/math/mean.py  >> $DIR/results/domain-threads-mean.dat
        cat $DIR/results/domain-threads-$num-$NUM_PAGES-* | $DIR/../scripts/math/stdev.py >> $DIR/results/domain-threads-std.dat
        echo $num >> $DIR/results/num-threads.dat
        echo "Processing with ${num} threads... done!"
    done

    echo "Generating plot..."
    python3 $DIR/../scripts/generate-plot.py \
	    --mean    $DIR/results/domain-threads-mean.dat \
	    --std     $DIR/results/domain-threads-std.dat \
	    --xvalues $DIR/results/num-threads.dat \
	    --ylabel  "pkey_protect latency (ns)" \
	    --xlabel  "Number of Threads" \
	    --plot    "domain_vary_threads"

    echo "All done!"
}

function access_vary_threads {
    NUM_PAGES=64000
    rm $DIR/results/access-threads-* &> /dev/null
    rm $DIR/results/num-threads.dat &> /dev/null
    for num in 1 2 3 4 5 6 7
    do
        echo "Processing with ${num} threads..."
        for i in $(seq 10)
        do
            $DIR/../benchmark access $num $NUM_PAGES >> $DIR/results/access-threads-$num-$NUM_PAGES-$i.log
        done

	echo "Calculating average and standard deviation..."
        cat $DIR/results/access-threads-$num-$NUM_PAGES-* | $DIR/../scripts/math/mean.py  >> $DIR/results/access-threads-mean.dat
        cat $DIR/results/access-threads-$num-$NUM_PAGES-* | $DIR/../scripts/math/stdev.py >> $DIR/results/access-threads-std.dat
        echo $num >> $DIR/results/num-threads.dat
        echo "Processing with ${num} threads... done!"
    done

    echo "Generating plot..."
    python3 $DIR/../scripts/generate-plot.py \
	    --mean    $DIR/results/access-threads-mean.dat \
	    --std     $DIR/results/access-threads-std.dat \
	    --xvalues $DIR/results/num-threads.dat \
	    --ylabel  "pkey_set latency (ns)" \
	    --xlabel  "Number of Threads" \
	    --plot    "access_vary_threads"

    echo "All done!"
}

function domain_vary_pages {
    NUM_THREADS=1
    rm $DIR/results/domain-pages-* &> /dev/null
    rm $DIR/results/num-pages.dat &> /dev/null
    for num in 4000 8000 16000 32000 64000 12800 256000
    do
        echo "Processing with ${num} pages..."
        for i in $(seq 1000)
        do
            $DIR/../benchmark domain $NUM_THREADS $num >> $DIR/results/domain-pages-$NUM_THREADS-$num-$i.log
        done

	echo "Calculating average and standard deviation..."
        cat $DIR/results/domain-pages-$NUM_THREADS-$num-* | $DIR/../scripts/math/mean.py  >> $DIR/results/domain-pages-mean.dat
        cat $DIR/results/domain-pages-$NUM_THREADS-$num-* | $DIR/../scripts/math/stdev.py >> $DIR/results/domain-pages-std.dat
        echo $num >> $DIR/results/num-pages.dat
        echo "Processing with ${num} pages... done!"
    done

    echo "Generating plot..."
    python3 $DIR/../scripts/generate-plot.py \
	    --mean    $DIR/results/domain-pages-mean.dat \
	    --std     $DIR/results/domain-pages-std.dat \
	    --xvalues $DIR/results/num-pages.dat \
	    --ylabel  "pkey_protect latency (ns)" \
	    --xlabel  "Number of (Contiguous) Pages" \
	    --plot    "domain_vary_pages"

    echo "All done!"
}

access_vary_threads
domain_vary_threads
domain_vary_pages
