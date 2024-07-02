#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
ITERS=10
JAR=$DIR/target/isolate-benchmark-0.1-jar-with-dependencies.jar
DOCKER_IMG=ghcr.io/graalvm/graalvm-ce:latest
DOCKER_SCRATCH_IMG=scratch-ni
RESULTS_DIR=$DIR/results

# TODO - update instructions to use the bridge.
# IP of your local host.
TAP_GW=194.210.229.99

# A free IP in your network.
TAP_IP=194.210.229.100

# Mask of your local network.
TAP_MASK=255.255.254.0

if [[ -z "${ARGO_HOME}" ]]; then
        echo "ARGO_HOME is not defined. Existing..."
        exit 1
fi

if [[ -z "${JAVA_HOME}" ]]; then
        echo "JAVA_HOME is not defined. Existing..."
        exit 1
fi

function log_rss {
    while sudo kill -0 $1 &> /dev/null; do
        ps -q $1 -o rss= >> $2
        sleep .5
    done
}

function parent_child_memory {
    parent_pid=$1

    total_rss=0
    total_pss=0

    parent_rss=$(ps -q $parent_pid -o rss=)
    parent_pss=$(cat /proc/$parent_pid/smaps | grep "^Pss:" | awk '{ sum += $2 } END { print sum }')
    total_rss=$((total_rss + parent_rss))
    total_pss=$((total_pss + parent_pss))

    for child_pid in $(ps -o pid --no-headers --ppid $parent_pid)
    do
        child_rss=$(ps -q $child_pid -o rss=)
    child_pss=$(cat /proc/$child_pid/smaps | grep "^Pss:" | awk '{ sum += $2 } END { print sum }')
    total_rss=$((total_rss + child_rss))
    total_pss=$((total_pss + child_pss))
    done

    echo "RSS $total_rss PSS $total_pss"
}

# TODO - do not depend on argo resources
function get_kernel {
    emulator=$1
    if [ "$emulator" = "firecracker" ]; then
        kernel=$ARGO_HOME/resources/hello-vmlinux.bin
    else
        kernel=$ARGO_HOME/resources/vmlinux-4.14.35-1902.6.6.1.el7.container
    fi
}

# TODO - update, do not benchmark niuk. Benchmark directly firecracker and qemu.
function vm_rss {
    emulator=$1
    get_kernel $emulator

    # Use a temporary directory.
    mkdir results/rss-$emulator &> /dev/null
    cd results/rss-$emulator
    rm rss-*.dat &> /dev/null

    $JAVA_HOME/bin/native-image -cp $JAR Sleep sleep-benchmark

    $ARGO_HOME/niuk/build_niuk.sh $JAVA_HOME $PWD/sleep-benchmark $PWD/sleep-benchmark.img

    sudo bash $ARGO_HOME/lambda-manager/src/scripts/create_taps.sh ttap $TAP_IP

    $ARGO_HOME/niuk/run_niuk.sh \
    --vmm $emulator \
    --disk $PWD/sleep-benchmark.img \
    --kernel $kernel \
    --memory 512 --cpu 1 \
    --ip $TAP_IP --gateway $TAP_GW --mask $TAP_MASK --tap ttap \
    --console &> emulator.log &

    # Note: give it time for the vm to be launched.
    sleep 1

    PID=$(ps -o pid --no-headers --ppid $(sudo cat lambda.pid))

    log_rss $PID rss-$emulator.dat &

    # Note: give it time to create samples.
    sleep 10

    sudo kill $PID
    wait

    sudo bash $ARGO_HOME/lambda-manager/src/scripts/remove_taps.sh ttap

    cd - &> /dev/null
}

# TODO - update
function vm_latency {
    emulator=$1
    get_kernel $emulator

    # Use a temporary directory.
    mkdir results/latency-$emulator &> /dev/null
    cd results/latency-$emulator
    rm latency-*.dat &> /dev/null

    $JAVA_HOME/bin/native-image -cp $JAR Time2 time-benchmark

    $ARGO_HOME/niuk/build_niuk.sh $JAVA_HOME $PWD/time-benchmark $PWD/time-benchmark.img

    sudo bash $ARGO_HOME/lambda-manager/src/scripts/create_taps.sh ttap $TAP_IP

    for i in $(seq 1 $ITERS)
    do
        # Note: this sleep is necessary to allow the tap state to be propagated.
        sleep 1
        ts=$(($(date +%s%N)/1000000))

        $ARGO_HOME/niuk/run_niuk.sh \
            --vmm $emulator \
            --disk $PWD/time-benchmark.img \
            --kernel $kernel \
            --memory 512 --cpu 1 \
            --ip $TAP_IP --gateway $TAP_GW --mask $TAP_MASK --tap ttap \
            --console &> emulator.log &

        # Note: give the vm time to boot.
        sleep 1

        # Note: lambda.pid points the parent process of both firecracker and qemu.
        sudo kill $(ps -o pid --no-headers --ppid $(sudo cat lambda.pid))

        tf=$(cat emulator.log | grep "\[ms since epoch\]" | awk '{print $NF}' | tr -d '\t\n\r')
        echo $(($tf - $ts)) >> latency-$emulator.dat
    done
    sudo bash $ARGO_HOME/lambda-manager/src/scripts/remove_taps.sh ttap

    cd - &> /dev/null
}

function firecracker_snapshot_rss_latency {
    # Use a temporary directory.
    rm -r $DIR/results/firecracker-snapshot &> /dev/null
    mkdir $DIR/results/firecracker-snapshot&> /dev/null
    cd $DIR/results/firecracker-snapshot
    ip=172.18.0.2

    for i in $(seq 1 $ITERS)
    do
        echo "(iter $i) Starting vm..."
        $DIR/../demos/firecracker/start-vm.sh $ip &> start-vm-$i-a.log &
        sleep 1
    echo "(iter $i) Configuring vm..."
        $DIR/../demos/firecracker/config-vm.sh $ip &> config-vm-$i.log
        sleep 1
    echo "(iter $i) Snapshotting vm..."
        $DIR/../demos/firecracker/snapshot-vm.sh $ip &> snapshot-vm-$i.log
        sleep 1
    echo "(iter $i) Stopping vm..."
        $DIR/../demos/firecracker/stop-vm.sh $ip &> stop-vm-$i-a.log
        sleep 1
    echo "(iter $i) Starting vm..."
        $DIR/../demos/firecracker/start-vm.sh $ip &> start-vm-$i-b.log &
        sleep 1
    echo "(iter $i) Restoring vm..."
        $DIR/../demos/firecracker/restore-vm.sh $ip &> restore-vm-$i.log
        sleep 1
    echo "(iter $i) Tracking memory..."
        pid=$(sudo fuser $DIR/../demos/firecracker/$ip/firecracker.socket 2>&1 | grep firecracker.socket | awk '{print $2}')
        log_rss $pid $DIR/results/firecracker-snapshot/rss-firecracker-snapshot-$i.dat &> /dev/null &
        sleep 5
    echo "(iter $i) Stopping vm..."
        $DIR/../demos/firecracker/stop-vm.sh $ip &> stop-vm-$i-b.log
        sleep 1
    echo "(iter $i) Deleting vm..."
        $DIR/../demos/firecracker/delete-vm.sh $ip &> delete-vm-$i.log
    done
    cd - &> /dev/null
}

function docker_rss_latency {
    rm -f $RESULTS_DIR/*-docker.dat
    docker run --rm $DOCKER_IMG sleep 10 &> /dev/null &
    log_rss $! $RESULTS_DIR/rss-docker.dat &> /dev/null
    for i in $(seq 1 $ITERS)
    do
        ts=$(date +%s%N)
        tf=$(docker run --rm $DOCKER_IMG date +%s%N)
        tt=$((($tf - $ts) / 1000000))
        echo $tt >> $RESULTS_DIR/latency-docker.dat

        ts=$(date +%s%N)
        did=$(docker create $DOCKER_IMG date +%s%N)
        tf=$(date +%s%N)
        tt=$((($tf - $ts) / 1000000))
        echo $tt >> $RESULTS_DIR/latency-create-docker.dat

        ts=$(date +%s%N)
        tf=$(docker start -i $did)
        tt=$((($tf - $ts) / 1000000))
        echo $tt >> $RESULTS_DIR/latency-start-docker.dat

        ts=$(date +%s%N)
        docker rm $did
        tf=$(date +%s%N)
        tt=$((($tf - $ts) / 1000000))
        echo $tt >> $RESULTS_DIR/latency-rm-docker.dat
    done
}

function docker_scratch_rss_latency {

    function generate_scratch_image {
        cd src/main/docker
        $JAVA_HOME/bin/native-image -cp $JAR --static Time2 time
        $JAVA_HOME/bin/native-image -cp $JAR --static Sleep sleep
        docker build --rm -t $DOCKER_SCRATCH_IMG .
        rm sleep time *.build_artifacts.txt
        cd - &> /dev/null
    }

    generate_scratch_image
    rm -f $RESULTS_DIR/*-scratch.dat
    docker run --rm $DOCKER_SCRATCH_IMG /sleep &> /dev/null &
    log_rss $! $RESULTS_DIR/rss-docker-scratch.dat &> /dev/null
    for i in $(seq 1 $ITERS)
    do
        ts=$(($(date +%s%N) / 1000000))
        latency=$(docker run --rm $DOCKER_SCRATCH_IMG /time)
        tf=$(echo $latency | grep "\[ms since epoch\]" | awk '{print $NF}' | tr -d '\t\n\r')
        tt=$(($tf - $ts))
        echo $tt >> $RESULTS_DIR/latency-docker-scratch.dat

        ts=$(date +%s%N)
        did=$(docker create $DOCKER_SCRATCH_IMG /time)
        tf=$(date +%s%N)
        tt=$((($tf - $ts) / 1000000))
        echo $tt >> $RESULTS_DIR/latency-create-docker-scratch.dat

        ts=$(($(date +%s%N) / 1000000))
        latency=$(docker start -i $did)
        tf=$(echo $latency | grep "\[ms since epoch\]" | awk '{print $NF}' | tr -d '\t\n\r')
        tt=$(($tf - $ts))
        echo $tt >> $RESULTS_DIR/latency-start-docker-scratch.dat

        ts=$(date +%s%N)
        docker rm $did
        tf=$(date +%s%N)
        tt=$((($tf - $ts) / 1000000))
        echo $tt >> $RESULTS_DIR/latency-rm-docker-scratch.dat
    done
}

function hotspot_rss_latency {
    rm -f $RESULTS_DIR/*-hotspot.dat
    java -cp $JAR Sleep &
    log_rss $! $RESULTS_DIR/rss-hotspot.dat &> /dev/null
    for i in $(seq 1 $ITERS)
    do
        java -cp $JAR Time $(($(date +%s%N)/1000000)) >> $RESULTS_DIR/latency-hotspot.dat
    done
}

function node_rss_latency {
    rm -f $RESULTS_DIR/*-node.dat
    node -e 'new Promise((resolve) => setTimeout(resolve, 5000)).then();' &
    log_rss $! $RESULTS_DIR/rss-node.dat &> /dev/null
    for i in $(seq 1 $ITERS)
    do
        stime=$(($(date +%s%N)/1000000))
        ftime=$(node -e 'console.log(Date.now());')
        echo $((ftime - $stime)) >> $RESULTS_DIR/latency-node.dat
    done
}

function cpython_rss_latency {
    rm -f $RESULTS_DIR/*-python.dat
    python -c "import time; time.sleep(5)" &
    log_rss $! $RESULTS_DIR/rss-python.dat &> /dev/null
    for i in $(seq 1 $ITERS)
    do
        stime=$(($(date +%s%N)/1000000))
        ftime=$(python -c "import time; print(round(time.time() * 1000))")
        echo $((ftime - $stime)) >> $RESULTS_DIR/latency-python.dat
    done
}

function graalvisor_rss_latency {
    rm $RESULTS_DIR/*-graalvisor.dat
    for i in $(seq 1 $ITERS)
    do
        export lambda_timestamp="$(date +%s%N | cut -b1-13)"
        $ARGO_HOME/graalvisor/build/native-image/polyglot-proxy &>> $RESULTS_DIR/latency-graalvisor.dat &
        pid=$!
        log_rss $! $RESULTS_DIR/rss-graalvisor.dat &> /dev/null &
        sleep 5
        kill $pid
    done
}

function graalvisor_sandbox_rss_latency {

    function build_ni_so {
        $JAVA_HOME/bin/native-image -cp $JAR:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
                -DGraalVisorGuest=true \
                -Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
                --initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
                -H:ConfigurationFileDirectories=../../src/main/resources/ni-agent-config \
                --shared \
                -H:Name=libapp
    }

    function measure_memory {
        # Launch graalvisor
        $ARGO_HOME/graalvisor/build/native-image/polyglot-proxy &> memory.log &
        pid=$!

        # Register application.
        curl -s -X POST 127.0.0.1:8080/register?name=sleep\&entryPoint=Sleep\&language=java\&sandbox=$sandbox -H 'Content-Type: application/json' --data-binary @libapp.so &> memory-app.log

        # Measuring RSS and PSS (in case of process sandbox).
        for i in $(seq 1 $ITERS)
        do
            bmem=$(parent_child_memory $pid)
            curl -s -X POST 127.0.0.1:8080 -H 'Content-Type: application/json' -d '{"name":"sleep","async":"false","cached":"false","arguments":"{\"millis\":\"2000\"}"}' &>> memory-app.log &
            curl_pid=$!

            # Let the request get started.
            sleep 1

            amem=$(parent_child_memory $pid)
            brss=$(echo $bmem | awk '{print $2}')
            arss=$(echo $amem | awk '{print $2}')
            bpss=$(echo $bmem | awk '{print $4}')
            apss=$(echo $amem | awk '{print $4}')
            echo "RSS memory=$((arss - brss))" >> rss-graalsivor.dat
            echo "PSS memory=$((apss - bpss))" >> pss-graalsivor.dat
            wait $curl_pid
        done

        # Kill graalvisor
        kill $pid &> /dev/null
    }

    function measure_latency {
        # Launch graalvisor
        $ARGO_HOME/graalvisor/build/native-image/polyglot-proxy &> latency.log &
        pid=$!

        # Register application.
        curl -s -X POST 127.0.0.1:8080/register?name=time\&entryPoint=Time\&language=java\&sandbox=$sandbox -H 'Content-Type: application/json' --data-binary @libapp.so &> latency-app.log

        # Measuring latency.
        for i in $(seq 1 $ITERS)
        do
            curl -s -X POST 127.0.0.1:8080 -H 'Content-Type: application/json' -d '{"name":"time","async":"false","cached":"false","arguments":"{\"stime\":\"0\"}"}' &>> latency-app.log
        done

        # Kill graalvisor
        kill $pid &> /dev/null
    }

    sandbox=$1

    # Use a temporary directory.
    rm -r $DIR/results/graalvisor-$sandbox &> /dev/null
    mkdir $DIR/results/graalvisor-$sandbox &> /dev/null
    cd $DIR/results/graalvisor-$sandbox

    # Build application into a shared library.
    build_ni_so

    measure_latency
    measure_memory

    cd - &> /dev/null
}

mvn package
vm_rss firecracker
vm_latency firecracker
vm_rss qemu
vm_latency qemu
firecracker_snapshot_rss_latency
docker_rss_latency
docker_scratch_rss_latency
graalvisor_rss_latency
graalvisor_sandbox_rss_latency process
graalvisor_sandbox_rss_latency process
hotspot_rss_latency
node_rss_latency
cpython_rss_latency
