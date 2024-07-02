#!/bin/bash

function process_class {
	echo "$1"
	javap -v -p $1 | grep -B 3 "ACC_NATIVE"
}
export -f process_class

# Extract dependencies.
mkdir target/shopcart-0.3.6
cp target/shopcart-0.3.6.jar target/shopcart-0.3.6
cd target/shopcart-0.3.6
jar xf shopcart-0.3.6.jar
cd -

# Process all class files.
find target/shopcart-0.3.6 -name "*.class" | wc -l
find target/shopcart-0.3.6 -name "*.class" | parallel --progress process_class {}
