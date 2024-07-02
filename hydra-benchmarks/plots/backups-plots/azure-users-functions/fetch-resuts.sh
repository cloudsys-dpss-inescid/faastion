#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

AZURE_DATASET_HOME=$DIR/../../../azure-dataset

DAY_ID=d03
FIRST_MIN=721
LAST_MIN=730
MAX_MEMORY=16384

cd $AZURE_DATASET_HOME

echo "Generating dataset..."
java -cp build/classes/java/main org.graalvm.argo.dataset.DatasetProcessor $DAY_ID $FIRST_MIN $LAST_MIN $MAX_MEMORY
echo "Generating dataset... done!"

INVOCATIONS_FILE=output/result_d03.csv
KEEP_ALIVE=120000
OUTPUT_FILE=plot-data-$DAY_ID-$FIRST_MIN-$LAST_MIN-$MAX_MEMORY-$KEEP_ALIVE.txt

echo "Running simulation..."
java -cp build/classes/java/main org.graalvm.argo.dataset.PlotDataGenerator $INVOCATIONS_FILE $OUTPUT_FILE $KEEP_ALIVE
echo "Running simulation... done!"

cd - &> /dev/null

echo "Copying output file..."
cp $AZURE_DATASET_HOME/output/plot-data-$DAY_ID-$FIRST_MIN-$LAST_MIN-$MAX_MEMORY-$KEEP_ALIVE.txt .
echo "Copying output file... done!"
