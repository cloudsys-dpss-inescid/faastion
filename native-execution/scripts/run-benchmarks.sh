#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
BENCHMARKS_DIR=$ARGO_HOME/benchmarks/src/java/SeBS
JAVA_AGENT=$DIR/../javassist/target/JavassistWrapper-1.0-jar-with-dependencies.jar
INSTRUMENTATION=$DIR/../instrumentation/target/BytecodeTransformer-1.0-jar-with-dependencies.jar
TOOL="MethodExecutionTimer"

function setup_classify {
    export FUNCTION_ID=classify
    export BENCHMARK_NAME=classify
    class_path=$BENCHMARKS_DIR/gv-$benchmark/build/libs/classify-1.0-all.jar
    entrypoint=com.classify.Classify
}

function setup_native-bfs {
    export FUNCTION_ID=bfs
    export BENCHMARK_NAME=bfs
    class_path=$BENCHMARKS_DIR/gv-$benchmark/build/libs/bfs-1.0-all.jar
    entrypoint=com.jni.BFS
}

function setup_native-compression {
    export FUNCTION_ID=zip
    export BENCHMARK_NAME=zip
    class_path=$BENCHMARKS_DIR/gv-$benchmark/build/libs/zip-1.0-all.jar
    entrypoint=com.jni.ZIPCompression
}

function setup_native-mst {
    export FUNCTION_ID=mst
    export BENCHMARK_NAME=mst
    class_path=$BENCHMARKS_DIR/gv-$benchmark/build/libs/mst-1.0-all.jar
    entrypoint=com.jni.MST
}

function setup_native-pagerank {
    export FUNCTION_ID=pagerank
    export BENCHMARK_NAME=pagerank
    class_path=$BENCHMARKS_DIR/gv-$benchmark/build/libs/pagerank-1.0-all.jar
    entrypoint=com.jni.PageRank
}

function setup_thumbnail {
    export FUNCTION_ID=thumbnail
    export BENCHMARK_NAME=thumbnail
    class_path=$BENCHMARKS_DIR/gv-$benchmark/build/libs/thumbnail-1.0-all.jar
    entrypoint=com.thumbnail.Thumbnail
}

rm -rf $DIR/results

for benchmark in classify native-bfs native-compression native-mst native-pagerank thumbnail
do
    setup_$benchmark

    echo "Running $benchmark..."
    rm -r -f $DIR/results/tmp
    mkdir -p $DIR/results/$benchmark $DIR/results/tmp
    cd $DIR/results/$benchmark

    # Run the java code with the agent: dynamic analysis
    CMD="$DEF_JAVA_HOME/bin/java -cp ${class_path} -javaagent:${JAVA_AGENT}=${TOOL}::output ${entrypoint}"
    ../../native-benchmark.py -t 4 -c "$CMD" &> native-benchmark.log

    # Run the java code with the agent: static analysis
    export SNIPPETS_DIR=$DIR/results/tmp
    native_calls=$($DEF_JAVA_HOME/bin/java -cp $INSTRUMENTATION org.faastion.javassist.BytecodeTransformer ${class_path} | grep 'Native' | wc -l)
    # echo "Number of native calls: $native_calls"

    # Capture statistics.
    cat native-benchmark.log | grep "Average percentage of native execution" | awk '{print $6}'         >> ../percentages.dat
    cat native-benchmark.log | grep "Total number of transitions per invocation" | awk '{print $7}'     >> ../transitions.dat
    cat native-benchmark.log | grep "Average function invocation time" | awk '{print $5}'               >> ../latencies.dat
    echo "$native_calls" >> ../native_calls.dat
    echo "$benchmark" >> ../benchmarks.dat
    cd - &> /dev/null
    echo "Running $benchmark... done!"
done
