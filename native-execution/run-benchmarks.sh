#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

JAVA_AGENT=$(DIR)/javassist/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="MethodExecutionTimer"

function build_javassist_agent {
    cd $(DIR)/javassist
    mvn package
    cd - &> /dev/null
}

function build_benchmark {
    cd $(DIR)/benchmarks/$benchmark
    source ./build.sh
    cd - &> /dev/null
}

build_javassist_agent

rm -rf $(DIR)/results

for benchmark in aesencryption zipcompression imagemanipulation 
	#helloworld filehashing httprequest shopcart petclinic classify
do
    build_benchmark

    echo "Running $benchmark..."
    mkdir -p $(DIR)/results/$benchmark
    cd $(DIR)/results/$benchmark

    # Get all directories with a *.class file in them and replace '/' with '.'
    output=$(jar -tf ${class_path} | grep -oE ".*\/[^/]+\.class" | sed 's#/[^/]*$##; s#/#.#g' | sort -u)
    # String of packages separated by commas
    printf -v packages '%s,' $output
    packages=${packages%,}

    # Run the java code with the agent.
    echo "java -cp ${class_path} -javaagent:${JAVA_AGENT}=${TOOL}:${packages}:output ${entrypoint}" > run.sh
    $(DIR)/../../native-benchmark.py -t 1 -c "java -cp ${class_path} -javaagent:${JAVA_AGENT}=${TOOL}:${packages}:output ${entrypoint}" &> native-benchmark.log

    # Capture statistics.
    cat native-benchmark.log | grep "Average percentage of native execution" | awk '{print $6}' >> ../percentages.dat
    cat native-benchmark.log | grep "Number of transitions per second" | awk '{print $6}'       >> ../transitions.dat
    echo "$benchmark" >> ../benchmarks.dat
    cd - &> /dev/null
    echo "Running $benchmark... done!"
done
