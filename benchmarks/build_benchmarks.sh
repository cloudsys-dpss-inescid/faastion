#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
SEBS_DIR=$DIR/src/java/SeBS

for benchmark in gv-classify gv-native-pagerank gv-native-bfs gv-native-mst gv-native-compression gv-uploader gv-dynamic-html gv-dna-visualization gv-thumbnail
do
	cd $SEBS_DIR/$benchmark
	./build_script.sh
	cd - &> /dev/null
done
