#!/bin/bash

function process_class {
	echo "$1"
	javap -v -p $1 | grep -B 3 "ACC_NATIVE"
}
export -f process_class

# Extract dependencies.
mkdir target/petclinic-jpa-0.1.6
cp target/petclinic-jpa-0.1.6.jar target/petclinic-jpa-0.1.6
cd target/petclinic-jpa-0.1.6
jar xf petclinic-jpa-0.1.6.jar
cd BOOT-INF/lib
for j in *.jar
do
	jar xf $j
done
cd ../../

# Process all class files.
find .  -name "*.class" | wc -l
find .  -name "*.class" | parallel --progress process_class {}
