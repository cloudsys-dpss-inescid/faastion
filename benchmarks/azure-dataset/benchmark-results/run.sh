#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

GRAALVISOR_HOME=$ARGO_HOME/graalvisor
BENCHMARKS_HOME=$ARGO_HOME/benchmarks
AZURE_DATASET_HOME=$BENCHMARKS_HOME/azure-dataset
JAVA_BENCHMARKS=$BENCHMARKS_HOME/src/java

LOGS_HOME=$(DIR)/logs
TRACES_HOME=$(DIR)/traces

GREEN='\033[0;32m'
NC='\033[0m' # No Color


function build_azure {
    cd $AZURE_DATASET_HOME
    ./build.sh
    ./download_dataset.sh
    cd -
}

function generate_dataset {
    # Uncomment if azure-dataset is first time running
    # build_azure
    $AZURE_DATASET_HOME/trace-generator.sh -m 2048 -d d01 -t $TRACES_HOME/gen-trace.csv
}

function extract_functions {
    python3 benchmark_assignment.py -f $num_functions -i $TRACES_HOME/gen-trace.csv -o $TRACES_HOME/trace.csv
}

function log_resources {
    PID=$1
    OFILE_RSS=/tmp/$approach/footprint.csv

    rm $OFILE_RSS &> /dev/null
        while kill -0 $PID &> /dev/null; do
                # The idea for memory is that we traverse the entire pid subprocess tree
                # and memory memory utilization. We sum all individual memory and return.
                s_mem=0
                timestamp=$(date -u +"%s%3N")
                for p in $(pstree -p $PID | grep -o '([0-9]\+)' | grep -o '[0-9]\+')
                do
                    p_mem=$(ps -q $p -o rss=)
                    s_mem=$((s_mem + p_mem))
                done
                echo "$s_mem,$timestamp" >> $OFILE_RSS
                sleep .100
        done
}

function start_webserver {
    cd $(DIR)/webserver
    nohup ./webserver.sh > $LOGS_HOME/webserver.log 2>&1 &
    cd -
}

function start_svm {
    export lambda_timestamp="$(date +%s%N | cut -b1-13)"
    export lambda_port="8080"
    
    # Faastion process pool requirement
    rm -rf /tmp/fifo
    mkdir -p /tmp/fifo
    
    export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:$LD_LIBRARY_PATH
    export LD_PRELOAD=$GRAALVISOR_HOME/build/libs/libpreload.so
    # Start Graalvisor
    $GRAALVISOR_HOME/build/native-image/polyglot-proxy &
    wait
    unset LD_PRELOAD
}

function compile_java_classes {
    ./gradlew clean shadowJar
}

function compile_jni_benchmarks {
    for benchmark in gv-native-factorization gv-native-hw-1 gv-native-matrixmul gv-file-hashing gv-httprequest gv-sleep #gv-classify gv-video-processing
    do
        cd "$JAVA_BENCHMARKS/$benchmark"
        ./build_script.sh
        cd -
    done
}

function compile_benchmarks {
    for benchmark in gv-native-hw-1 gv-native-factorization gv-native-matrixmul
    do
        cd "$JAVA_BENCHMARKS/$benchmark"
        ./build_script_proc_iso.sh
        cd -
    done
}

function execute {
    # Start Graalvisor
    start_svm &> logs/lambda.log &
    PID=$(echo -n "$!")

    # Log Resources (memory and CPU)
    log_resources $PID &

    JAR=$(DIR)/build/libs/trace-executor-1.0-all.jar
    MAIN=ExecutorEntryPoint
    
    $JAVA_HOME/bin/java -cp $JAR $MAIN \
        -i      $TRACES_HOME/trace.csv  \
        -nf     $num_functions \
        -a      $approach \
        -d
    
    # Kill Graalvisor and Webserver
    pkill -9 -f polyglot-proxy
}

function cleanup_resources {
    echo "Received signal, cleaning up resources..."
    pkill -9 -f polyglot-proxy
    pkill -9 -f webserver.sh
    exit
}

# Preparing global paths.
if [ -z "${ARGO_HOME}" ]; then
    echo "ARGO_HOME is not defined. Existing..."
    exit 1
fi
if [ "$#" -ne 1 ]; then
    echo "Syntax: <# of applications>"
    exit 1
else
    num_functions=$1
fi

# Clean resources if killed with signal
trap 'cleanup_resources' SIGINT

rm -rf $LOGS_HOME
mkdir -p $LOGS_HOME
mkdir -p $TRACES_HOME

### SCRIPT STARTS HERE ###
generate_dataset
extract_functions
compile_java_classes

start_webserver

# TODO Make average logic!!!

# JNI compilation
compile_jni_benchmarks

for approach in faastlane #faastion 
do
    echo -e "${GREEN}###################################################"
    echo -e "#          Measuring metrics for $approach         #"
    echo -e "###################################################${NC}"
    
    rm -rf /tmp/$approach
    mkdir -p /tmp/$approach

    [ "$approach" = "faastlane" ] && export EAGER_MPK=1
    execute
    [ "$approach" = "faastlane" ] && unset EAGER_MPK
done

# Normal compilation
compile_benchmarks

for approach in process isolate 
do
    echo -e "${GREEN}###################################################"
    echo -e "#          Measuring metrics for $approach         #"
    echo -e "###################################################${NC}"

    rm -rf /tmp/$approach
    mkdir -p /tmp/$approach

    execute
done

# Generate plots
python3 plots.py

# Calculate throughput
workload=$(wc -l < traces/trace.csv)
throughput=$((workload / 600))
echo -e "Throughput ${throughput} invocations/s"

# Clean resources when finished
cleanup_resources