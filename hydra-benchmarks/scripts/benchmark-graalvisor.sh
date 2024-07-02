#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

source $(DIR)/shared.sh
source $(DIR)/benchmarks.sh

if [ "$#" -ne 4 ]; then
    echo "Syntax: <svm|container|vm> <app> <mode> <# of tests or concurrency level>"
    echo "Available backends: svm (Native Image), container (Docker container), vm (Firecracker VM)."
    echo "Available apps: $GV_BENCHMARKS"
    echo "Available modes: test benchmark. Test will perform a number of requests. Benchmark will use apache bench with the desired concurrency level."
    echo "Example: benchmark-graalvisor.sh svm gv_java_hw test 1"
    echo "Available environment variables: "
    echo "- SANDBOX=<isolate|runtime|process|context> - isolation level of concurrent requests in graalvisor. Defaults to isolate or context depending on the function language;"
    echo "- SNAPSHOT=<path> - path on disk where the snapshot should be stored/loaded from; Defaults to empty which leads to no snapshot being stored/loaded;"
    echo "- ITERATIONS=<number> - number of iterations the workload is ran. Defaults to 1;"
    echo "- WMULTIPLIER=<number> - workload multiplier to scale up or down the length of the benchmark (only used in benchmark mode). Defaults to 256;"
    echo "- WARMUP=<number> - number of warmup requests sent to graalvisor. Only used in benchmark mode. Defaults to zero;"
    echo "- CGROUP=<name> - name of the cgroup to use. A directory with that name will be created under /sys/fs/cgroup. Defaults to empty which leads to no CGROUP being used;"
    echo "- CGROUP_CPU_QUOTA=<number> - CPU quota for a 100ms period. A quota 100000 means using a full core. Only used if CGROUP is set. Defaults to 100000 (full core)."
    echo "- CGROUP_MEM=<number> - memory limit in MBs configued in the CGROUP. Only used if CGROUP is set. Defaults to 2048MBs."
    echo "- VM_CPU=<number> - number of cores that the VM will create internally (only used in vm mode). Defaults to 1;"
    echo "- VM_MEM=<number> - memory given to the VM (only used in vm mode). Defaults to 2048;"
    echo "- PIN_CORE=<boolean> - if true, will pin the process to core 0. Defaults to false;"
    echo "- DISABLE_TURBO=<boolean> - if true, will disable turbo boost. Defaults to false;"
    exit 1
else
    backend=$1
    app=$2
    mode=$3
    workload=$4
fi

function benchmark {
    if [ -z "$WMULTIPLIER" ]; then
        WMULTIPLIER=256
    fi

    if [ ! -z "$WARMUP" ]; then
            ab -p $APP_POST -T application/json -c 1 -n $WARMUP http://$ip:8080/warmup &> $tmpdir/ab-warmup.log
    fi

    ab -p $APP_POST -T application/json -c $workload -n $((workload * WMULTIPLIER)) http://$ip:8080/ &> $tmpdir/ab-init.log
    ab -p $APP_POST -T application/json -c $workload -n $((workload * WMULTIPLIER)) http://$ip:8080/ &> $tmpdir/ab.log
}

function test {
    for i in $(seq 1 $workload)
    do
        pretime
        curl -s -X POST $ip:8080 -H 'Content-Type: application/json' -d $(cat $APP_POST)
        postime
    done
}

function parallel {
    for i in $(seq 1 $workload)
    do
        curl -s -X POST $ip:8080 -H 'Content-Type: application/json' -d $(cat $APP_POST) &
    done
    wait
}

function run {
    # Setting up environment.
    if [ "$backend" == "container" ]; then
        ip=127.0.0.1
        start_container &> $tmpdir/lambda.log &
    elif [ "$backend" == "svm" ]; then
        ip=127.0.0.1
        start_svm &> $tmpdir/lambda.log &
    elif [ "$backend" == "vm" ]; then
        # Note: ip is already set when loading shared.sh
        start_vm &> $tmpdir/lambda.log &
    fi

    # Let the lambda start.
    wait_port $ip 8080

    # Get PID of lambda.
    if [ "$backend" == "container" ]; then
        PID=$(docker inspect --format '{{ .State.Pid }}' gcontainer)
    elif [ "$backend" == "svm" ]; then
        PID=$(cat $tmpdir/lambda.pid)
    elif [ "$backend" == "vm" ]; then
        PID=$(cat $tmpdir/lambda.pid)
    fi

    # Write lambda pid to file.
    echo -n "$PID" > $tmpdir/lambda.pid

    # Log memory.
    log_rss $PID $tmpdir/lambda.rss &

    # Prepares the local resources (cgroups, core pinning, turbo boost).
    prepare_resources

    # Load function into runtime.
    $app

    # Run test/benchmark.
    $mode | tee -a $tmpdir/app.log

    # Teardown the lambda.
    if [ "$backend" == "container" ]; then
        stop_container
    elif [ "$backend" == "svm" ]; then
        stop_svm
    elif [ "$backend" == "vm" ]; then
        stop_vm
    fi
    wait

    # Teardown local resource changes (delete cgroups, enable turbo boost).
    teardown_resources
}

# Setting a sandbox if not already set.
if [ -z "$SANDBOX" ]
then
    if [[ $app == *"_java_"* ]]; then
        export SANDBOX=isolate
    else
        export SANDBOX=context
    fi
fi

echo "Running environment=$backend; sandbox=$SANDBOX; app=$app; mode=$mode; workload=$workload; cpu=$VM_CPU; mem=$VM_MEM"
echo "Logs available at $tmpdir..."

# Writing post file to disk
APP_POST=$tmpdir/payload-rodrigo.post

# Preparing working directory
rm -r $tmpdir/ &> /dev/null
mkdir $tmpdir &> /dev/null

for iter in $(seq 1 $ITERATIONS)
do
    # Run...
    run
    # Preparing results directory
    results_dir=$BENCHMARKS_HOME/results/$APP_LANG/$APP_NAME-$backend-$SANDBOX-$mode-$workload-$VM_CPU-$VM_MEM/$iter
    mkdir -p $results_dir &> /dev/null
    cp $tmpdir/{*.log,*.rss} $results_dir &> /dev/null
    echo "Saved logs (iteration $iter): $results_dir/lambda.log"
done
