#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
JAVA_AGENT=$DIR/javassist/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="MethodExecutionTimer"

function build_javassist_agent {
    cd $DIR/javassist
    mvn package
    cd - &> /dev/null
}

function build_benchmark {
    cd $DIR/benchmarks/$benchmark
    ./build.sh
    cd - &> /dev/null
}

#build_javassist_agent

rm -r $DIR/results &> /dev/null

for benchmark in classify filehashing helloworld httprequest videoprocessing
do
    #build_benchmark

    echo "Running $benchmark..."
    mkdir -p $DIR/results/$benchmark
    cd $DIR/results/$benchmark

    BUILD=$DIR/benchmarks/$benchmark/build
    MANIFEST=${BUILD}/tmp/shadowJar/MANIFEST.MF
    CLASS_PATH=(${BUILD}/libs/*all.jar)

    # Replace * with the actual package name
    class_path=$(echo "${CLASS_PATH[*]}")
    entrypoint=$(grep -i 'Main-Class' ${MANIFEST} | awk -F ': ' '{print $2}')

    # Get all directories with a *.class file in them and replace '/' with '.'
    output=$(jar -tf ${class_path} | grep -oE ".*\/[^/]+\.class" | sed 's#/[^/]*$##; s#/#.#g' | sort -u)
    # String of packages separated by commas
    printf -v packages '%s,' $output
    packages=${packages%,}

    # Run the java code with the agent.
    $DIR/native-benchmark.py -c "java -cp ${class_path} -javaagent:${JAVA_AGENT}=${TOOL}:${packages}:output ${entrypoint}" &> $DIR/results/$benchmark/native-benchmark.log

    # Capture statistics.
    cat $DIR/results/$benchmark/native-benchmark.log | grep "Average percentage of native execution" | awk '{print $6}' >> $DIR/results/percentages.dat
    cat $DIR/results/$benchmark/native-benchmark.log | grep "Number of transitions per second" | awk '{print $6}' >> $DIR/results/transitions.dat
    echo "$benchmark" >> $DIR/results/benchmarks.dat
    cd - &> /dev/null
    echo "Running $benchmark... done!"
done
