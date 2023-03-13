#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

JAVA_AGENT=$DIR/javassist/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="MethodExecutionTimer"

PACKAGE=$1
ENTRYPOINT=$2

# Get all directories with a *.class file in them and replace '/' with '.'
packages=$(jar -tf ${PACKAGE} | grep -oE ".*\/[^/]+\.class" | sed 's#/[^/]*$##; s#/#.#g' | sort -u)

# String of packages separated by commas
printf -v output '%s,' $packages
output=${output%,}

echo "java -cp ${PACKAGE} -javaagent:${JAVA_AGENT}=${TOOL}:${output}:output ${ENTRYPOINT}"
