#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

AZURE_DATASET_HOME=$DIR/../../../azure-dataset

for day in "d01" "d02" "d03"
do
    java -cp $AZURE_DATASET_HOME/build/classes/java/main/ org.graalvm.argo.dataset.DatasetStatistics $AZURE_DATASET_HOME/input/invocations_per_function_md.anon.$day.csv > $DIR/avg_invocations_$day.dat
    cat $DIR/avg_invocations_$day.dat | grep User     | awk '{print $6}' > avg_user_invocations_$day.dat
    cat $DIR/avg_invocations_$day.dat | grep Function | awk '{print $6}' > avg_func_invocations_$day.dat
done
