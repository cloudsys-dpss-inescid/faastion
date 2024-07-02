#!/bin/bash

function process_class {
	echo "$1"
	javap -v -p $1 | grep -B 3 "ACC_NATIVE"
}
export -f process_class

# Extract dependencies.
mkdir build/filehashing-1.0-all
cp build/libs/filehashing-1.0-all.jar build/filehashing-1.0-all
cd build/filehashing-1.0-all
jar xf filehashing-1.0-all.jar
cd -

# Process all class files.
find build/filehashing-1.0-all -name "*.class" | wc -l
find build/filehashing-1.0-all -name "*.class" | parallel --progress process_class {}
