#!/bin/bash

function process_class {
	echo "$1"
	javap -v -p $1 | grep -B 3 "ACC_NATIVE"
}
export -f process_class

# Extract dependencies.
mkdir build/hello-world-1.0-all
cp build/libs/hello-world-1.0-all.jar build/hello-world-1.0-all
cd build/hello-world-1.0-all
jar xf hello-world-1.0-all.jar
cd -

# Process all class files.
find build/hello-world-1.0-all -name "*.class" | wc -l
find build/hello-world-1.0-all -name "*.class" | parallel --progress process_class {}
