#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

rm $DIR/results/*

# run webserver script in a subprocess
#(source ${DIR}/benchmarks/webserver)

for benchmark in ${DIR}/benchmarks/*/; do
    cmd=$(source ${DIR}/javassist-cmd.sh ${benchmark}build)

    # read the input file and store the output in a variable
    results=$(python3 native-benchmark.py -c "$cmd" -t 1)
    
    echo "$results" | grep "Average percentage of native execution" | awk '{print $6}' >> $DIR/results/percentages.dat &
    echo "$results" | grep "Number of transitions per second" | awk '{print $6}' >> $DIR/results/transitions.dat &
    basename "$benchmark" >> $DIR/results/benchmarks.dat &
done

python3 generate-plot.py