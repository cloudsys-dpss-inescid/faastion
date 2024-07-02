#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

source $(DIR)/benchmarks.sh

function test_gv_benchmarks {
    TEST_SET=""
    read -p "Test Graalvisor's Java benchmarks (y or Y, everything else as no)? " -n 1 -r
    echo    # move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
        TEST_SET="$TEST_SET $JV_GV_BENCHMARKS"
    fi
    read -p "Test Graalvisor's Python benchmarks (y or Y, everything else as no)? " -n 1 -r
    echo    # move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
        TEST_SET="$TEST_SET $PY_GV_BENCHMARKS"
    fi
    read -p "Test Graalvisor's JavaScript benchmarks (y or Y, everything else as no)? " -n 1 -r
    echo    # move to a new line
    if [[ $REPLY =~ ^[Yy]$ ]]
    then
        TEST_SET="$TEST_SET $JS_GV_BENCHMARKS"
    fi

    for benchmark in $TEST_SET
    do
        for backend in "svm" "container" "vm"
        do
            for sandbox in "default" "runtime" "process"
            do
                if [ "$sandbox" == "default" ]; then
                    unset SANDBOX
                else
                    export SANDBOX=$sandbox;
                fi
                $(DIR)/benchmark-graalvisor.sh $backend $benchmark test 1 | grep "\"result\""
           done
        done
    done
    unset SANDBOX
}

function cdf_latency_filehashing {
    export ITERATIONS=5
    $(DIR)/benchmark-cruntime.sh   container cr_java_filehashing test 25
    $(DIR)/benchmark-graalvisor.sh container gv_java_filehashing test 25
    unset ITERATIONS
}

# Memory (fixed HW resources of 1 core and 2GB of memory, measure ops/s/mb)
function efficiency {

    # Graalvisor
    function efficiency_gv {

         function efficiency_gv_java_single {
            export WMULTIPLIER=10000; $(DIR)/benchmark-graalvisor.sh vm gv_java_hw benchmark 1; unset WMULTIPLIER
            export WMULTIPLIER=5000; $(DIR)/benchmark-graalvisor.sh vm gv_java_filehashing benchmark 1; unset WMULTIPLIER
            export WMULTIPLIER=5000; $(DIR)/benchmark-graalvisor.sh vm gv_java_httprequest benchmark 1; unset WMULTIPLIER
            export WMULTIPLIER=2; $(DIR)/benchmark-graalvisor.sh vm gv_java_videoprocessing benchmark 1; unset WMULTIPLIER
            export WMULTIPLIER=10; $(DIR)/benchmark-graalvisor.sh vm gv_java_classify benchmark 1; unset WMULTIPLIER
            export WMULTIPLIER=10000; $(DIR)/benchmark-graalvisor.sh vm gv_java_shopcart benchmark 1; unset WMULTIPLIER
        }

        function efficiency_gv_java {
            export WMULTIPLIER=1000; $(DIR)/benchmark-graalvisor.sh vm gv_java_hw benchmark 8; unset WMULTIPLIER
            export WMULTIPLIER=1000; $(DIR)/benchmark-graalvisor.sh vm gv_java_filehashing benchmark 8; unset WMULTIPLIER
            export WMULTIPLIER=1000; $(DIR)/benchmark-graalvisor.sh vm gv_java_httprequest benchmark 8; unset WMULTIPLIER
            export WMULTIPLIER=1; $(DIR)/benchmark-graalvisor.sh vm gv_java_videoprocessing benchmark 1; unset WMULTIPLIER
            # Note: there is a bug in gv, it can't run 2 parallel calls to classify.
            # Since the workload is throughput intensive, having a second one would keep the same throughput so it is fine...
	    export WMULTIPLIER=5; $(DIR)/benchmark-graalvisor.sh vm gv_java_classify benchmark 1; unset WMULTIPLIER
            export WMULTIPLIER=5000; $(DIR)/benchmark-graalvisor.sh vm gv_java_shopcart benchmark 8; unset WMULTIPLIER
        }

        function efficiency_gv_javascript {
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_hw benchmark 8
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_dynamichtml benchmark 8
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_uploader benchmark 8
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_thumbnail benchmark 4
        }

        function efficiency_gv_python {
            $(DIR)/benchmark-graalvisor.sh vm gv_python_hw benchmark 8
            export WMULTIPLIER=5;
            $(DIR)/benchmark-graalvisor.sh vm gv_python_dynamichtml benchmark 4
            $(DIR)/benchmark-graalvisor.sh vm gv_python_thumbnail benchmark 2
            $(DIR)/benchmark-graalvisor.sh vm gv_python_uploader benchmark 4
            $(DIR)/benchmark-graalvisor.sh vm gv_python_compression benchmark 4
            export WMULTIPLIER=2;
            $(DIR)/benchmark-graalvisor.sh vm gv_python_videoprocessing benchmark 2
            export WMULTIPLIER=50;
            $(DIR)/benchmark-graalvisor.sh vm gv_python_mst benchmark 2
            unset WMULTIPLIER
        }

        # All gv benchmarks run on a single core.
        for sandbox in "isolate" "runtime" "process"
        do
            export SANDBOX=$sandbox
            #efficiency_gv_java_single
            efficiency_gv_java
            unset SANDBOX
        done
        for sandbox in "context" "process"
        do
            export SANDBOX=$sandbox
            # Used only for process sandbox, ignored for context.
            export WARMUP=1
            efficiency_gv_javascript
            efficiency_gv_python
            unset WARMUP
            unset SANDBOX
        done

    }

    # Graalvisor with vm snapshotting
    function efficiency_gv_snapshot {

        function efficiency_gv_java {
            snapshots=/tmp/snapshots/java
            mkdir -p $snapshots

            # VM memory and cgroup cpu quota for the following benchmarks.
            export VM_MEM=256
            export CGROUP_CPU_QUOTA=12500 # .125 cores

            # Hello world
            export SNAPSHOT=$snapshots/hw
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_java_hw test 1"
            export WMULTIPLIER=2000; $(DIR)/benchmark-graalvisor.sh vm gv_java_hw benchmark 1; unset WMULTIPLIER
            unset SNAPSHOT

            # File hashing
            export SNAPSHOT=$snapshots/fh
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_java_filehashing test 1"
            $(DIR)/benchmark-graalvisor.sh vm gv_java_filehashing benchmark 1
            unset SNAPSHOT

            # Http request
            export SNAPSHOT=$snapshots/rest
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_java_httprequest test 1"
            $(DIR)/benchmark-graalvisor.sh vm gv_java_httprequest benchmark 1
            unset SNAPSHOT

            # Shopcart
            export SNAPSHOT=$snapshots/shopcart
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_java_shopcart test 1"
            export WMULTIPLIER=1000; $(DIR)/benchmark-graalvisor.sh vm gv_java_shopcart benchmark 1; unset WMULTIPLIER
            unset SNAPSHOT

            # Video processing
            export SNAPSHOT=$snapshots/video
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            export VM_MEM=1024
            export CGROUP_CPU_QUOTA=50000 # .5 cores
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_java_videoprocessing test 1"
            export WMULTIPLIER=2; $(DIR)/benchmark-graalvisor.sh vm gv_java_videoprocessing benchmark 1; unset WMULTIPLIER
            unset CGROUP_CPU_QUOTA
            unset VM_MEM
            unset SNAPSHOT

            # Classify
            export SNAPSHOT=$snapshots/classify
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            export VM_MEM=2048
            export CGROUP_CPU_QUOTA=50000 # 1 core?
            # Tensorflow cannot be loaded two! We use test 0 because of that.
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_java_classify test 0"
            export WMULTIPLIER=10; $(DIR)/benchmark-graalvisor.sh vm gv_java_classify benchmark 1; unset WMULTIPLIER
            unset CGROUP_CPU_QUOTA
            unset VM_MEM
            unset SNAPSHOT
        }

        function efficiency_gv_javascript {
            snapshots=/tmp/snapshots/javascript
            mkdir -p $snapshots

            # VM memory and cgroup cpu quota for the following benchmarks.
            export VM_MEM=256
            export CGROUP_CPU_QUOTA=12500 # .125 cores

            # Hello world
            export SNAPSHOT=$snapshots/hw
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_javascript_hw test 100"
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_hw benchmark 1
            unset SNAPSHOT

            # Dynamic HTML
            export SNAPSHOT=$snapshots/html
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_javascript_dynamichtml test 100"
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_dynamichtml benchmark 1
            unset SNAPSHOT

            # Uploader
            export SNAPSHOT=$snapshots/uploader
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_javascript_uploader test 100"
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_uploader benchmark 1
            unset SNAPSHOT

            # Thumbnail
            export SNAPSHOT=$snapshots/thumbnail
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            export VM_MEM=512
            export CGROUP_CPU_QUOTA=25000 # .25 cores
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_javascript_thumbnail test 100"
            $(DIR)/benchmark-graalvisor.sh vm gv_javascript_thumbnail benchmark 1
            unset CGROUP_CPU_QUOTA
            unset VM_MEM
            unset SNAPSHOT
        }

        function efficiency_gv_python {
            snapshots=/tmp/snapshots/python
            mkdir -p $snapshots

            # Hello world
            export SNAPSHOT=$snapshots/hw
            export VM_MEM=256
            export CGROUP_CPU_QUOTA=12500 # .125 cores
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_python_hw test 100"
            $(DIR)/benchmark-graalvisor.sh vm gv_python_hw benchmark 1
            unset CGROUP_CPU_QUOTA
            unset VM_MEM
            unset SNAPSHOT

            # VM memory, cgroup cpu quota, and multiplier for the following benchmarks.
            export VM_MEM=512
            export CGROUP_CPU_QUOTA=25000 # .25 cores
            export WMULTIPLIER=5

            # Dynamic html
            export SNAPSHOT=$snapshots/html
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_python_dynamichtml test $WMULTIPLIER"
            $(DIR)/benchmark-graalvisor.sh vm gv_python_dynamichtml benchmark 1
            unset SNAPSHOT

            # Uploader
            export SNAPSHOT=$snapshots/uploader
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_python_uploader test $WMULTIPLIER"
            $(DIR)/benchmark-graalvisor.sh vm gv_python_uploader benchmark 1
            unset SNAPSHOT

            # Compression
            export SNAPSHOT=$snapshots/compression
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_python_compression test $WMULTIPLIER"
            $(DIR)/benchmark-graalvisor.sh vm gv_python_compression benchmark 1
            unset SNAPSHOT

            # VM memory, cgroup cpu quota for the following benchmarks.
            export VM_MEM=1024
            export CGROUP_CPU_QUOTA=50000 # .5 cores

            # Thumbnail
            export SNAPSHOT=$snapshots/thumbnail
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_python_thumbnail test $WMULTIPLIER"
            $(DIR)/benchmark-graalvisor.sh vm gv_python_thumbnail benchmark 1
            unset SNAPSHOT

            # MST
            export SNAPSHOT=$snapshots/mst
            export WMULTIPLIER=50
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_python_mst test $WMULTIPLIER"
            $(DIR)/benchmark-graalvisor.sh vm gv_python_mst benchmark 1

            # Video processing
            export SNAPSHOT=$snapshots/video
            export WMULTIPLIER=2;
            export CGROUP_CPU_QUOTA=100000 # 1 core
            export VM_MEM=2048
            sudo rm $SNAPSHOT.{disk,mem,snap} &> /dev/null
            bash -c "export ITERATIONS=1; $(DIR)/benchmark-graalvisor.sh vm gv_python_videoprocessing test $WMULTIPLIER"
            $(DIR)/benchmark-graalvisor.sh vm gv_python_videoprocessing benchmark 1
            unset SNAPSHOT

            unset SNAPSHOT
            unset WMULTIPLIER
            unset VM_MEM
            unset CGROUP_CPU_QUOTA
        }

        export SANDBOX="isolate"; efficiency_gv_java; unset SANDBOX
        export SANDBOX="context"; efficiency_gv_javascript; unset SANDBOX
        export SANDBOX="context"; efficiency_gv_python; unset SANDBOX
    }

    # Openwhisk runtimes
    function efficiency_cr {

        # VM memory and cgroup cpu quota for the following benchmarks.
        export VM_MEM=256
        export CGROUP_CPU_QUOTA=12500 # .125 cores

        $(DIR)/benchmark-cruntime.sh vm cr_java_hw benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_python_hw benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_javascript_hw benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_java_filehashing benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_javascript_dynamichtml benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_python_dynamichtml benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_python_thumbnail benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_javascript_uploader benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_java_httprequest benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_python_uploader benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_python_compression benchmark 1

        # VM memory and cgroup cpu quota for the following benchmarks.
        export VM_MEM=512
        export CGROUP_CPU_QUOTA=25000 # .25 cores

        $(DIR)/benchmark-cruntime.sh vm cr_python_videoprocessing benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_javascript_thumbnail benchmark 1
        $(DIR)/benchmark-cruntime.sh vm cr_python_mst benchmark 1

        export VM_MEM=1024
        export CGROUP_CPU_QUOTA=50000 # .5 cores
        $(DIR)/benchmark-cruntime.sh vm cr_java_videoprocessing benchmark 1
        unset CGROUP_CPU_QUOTA
        unset VM_MEM

        export VM_MEM=2048
        export CGROUP_CPU_QUOTA=100000 # 1 core
        $(DIR)/benchmark-cruntime.sh vm cr_java_classify benchmark 1
        unset CGROUP_CPU_QUOTA
        unset VM_MEM
    }

    # Photons runtimes
    function efficiency_ph {
        $(DIR)/benchmark-cruntime.sh vm ph_java_hw benchmark 8
        $(DIR)/benchmark-cruntime.sh vm ph_java_filehashing benchmark 8
        $(DIR)/benchmark-cruntime.sh vm ph_java_httprequest benchmark 8
        export WMULTIPLIER=2; $(DIR)/benchmark-cruntime.sh vm ph_java_videoprocessing benchmark 2; unset WMULTIPLIER
        export WMULTIPLIER=10;$(DIR)/benchmark-cruntime.sh vm ph_java_classify benchmark 1; unset WMULTIPLIER
    }

    export ITERATIONS=5
    export CGROUP="experiments"
    export PIN_CORE="true"
    # Disabling turbo will make some benchmarks more stable. However, it will make everything much slower.
    export DISABLE_TURBO="false"

    efficiency_gv
    efficiency_gv_snapshot
    efficiency_cr
    efficiency_ph

    # Clear variables.
    unset ITERATIONS
    unset CGROUP
    unset PIN_CORE
    unset DISABLE_TURBO
}

function startup_latency {

    function startup_latency_gv {
        for mode in svm vm;
        do
            for i in $(seq 1 10);
            do
                $(DIR)/benchmark-graalvisor.sh $mode gv_java_hw test 1
                cat /tmp/test-proxy/lambda.log | grep "Polyglot Lambda boot time"
            done
        done
    }

    function startup_latency_cr {
        if [[ -z "${FIRECRACKER_CONTAINERD_HOME}" ]]; then
            echo "FIRECRACKER_CONTAINERD_HOME is not defined."
            echo "Run export FIRECRACKER_CONTAINERD_HOME=/home/$USER/git/firecracker-containerd ?"
            echo "Exiting..."
            exit 1
        fi

        JS_IMG="docker.io/rfbpb/action-nodejs-v14:latest"
        PY_IMG="docker.io/rfbpb/action-python-v3.7:latest"
        JV_IMG="docker.io/rfbpb/java8action:latest"

        # JS, PY, JV on custom runtime.
        for img in $JS_IMG $PY_IMG $JV_IMG;
        do
            $FIRECRACKER_CONTAINERD_HOME/demo/firecracker-ctr.sh run --snapshotter devmapper --runtime aws.firecracker --tty --net-host $img vm1 &
            wait_port $ip 8080
            $FIRECRACKER_CONTAINERD_HOME/demo/firecracker-ctr.sh task kill -a vm1
            wait
            for i in $(seq 1 10);
            do
                echo "Starting $img at $(($(date +%s%N)/1000000)) ms"
                $FIRECRACKER_CONTAINERD_HOME/demo/firecracker-ctr.sh task start vm1 &
                wait_port $ip 8080
                $FIRECRACKER_CONTAINERD_HOME/demo/firecracker-ctr.sh task kill -a vm1
                wait
            done
            $FIRECRACKER_CONTAINERD_HOME/demo/firecracker-ctr.sh container del vm1
        done

        # JS vanilla, extract from virt-bench
        # PY vanilla, extract from virt-bench
        # JV vanilla, extract from virt-bench
    }

    startup_latency_gv
    startup_latency_cr
}

read -p "Run basic graalvisor tests (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    test_gv_benchmarks
    exit 0
fi

read -p "Run cdf latency experiment (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    cdf_latency_filehashing
    exit 0
fi

read -p "Run warm latency experiment (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    warm_latency
    exit 0
fi

read -p "Run startup latency experiment (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    startup_latency
    exit 0
fi

read -p "Run efficiency experiment (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    efficiency
    exit 0
fi

