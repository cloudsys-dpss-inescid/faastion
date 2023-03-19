#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
JAVA_AGENT=$DIR/javassist/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="MethodExecutionTimer"

BUILD=$1
MANIFEST=${BUILD}/tmp/jar/MANIFEST.MF
CLASS_PATH=(${BUILD}/libs/*all.jar)

# Replace * with the actual package name
class_path=$(echo "${CLASS_PATH[*]}")
entrypoint=$(grep -i 'Main-Class' ${MANIFEST} | awk -F ': ' '{print $2}')

# Get all directories with a *.class file in them and replace '/' with '.'
output=$(jar -tf ${class_path} | grep -oE ".*\/[^/]+\.class" | sed 's#/[^/]*$##; s#/#.#g' | sort -u)
# String of packages separated by commas
printf -v packages '%s,' $output
packages=${packages%,}

echo "java -cp ${class_path} -javaagent:${JAVA_AGENT}=${TOOL}:${packages}:output ${entrypoint}"
