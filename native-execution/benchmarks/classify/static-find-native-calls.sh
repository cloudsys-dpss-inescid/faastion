#!/bin/bash

function process_class {
	echo "$1"
	javap -v -p $1 | grep -B 3 "ACC_NATIVE"
}
export -f process_class

# Extract dependencies.
cd build/libs
jar xf classify-1.0-all.jar
cd -

# Process all class files.
find build/libs -name "*.class" | wc -l
find build/libs -name "*.class" | parallel --progress process_class {}
