#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
LOG=$DIR/build_benchmarks.log

JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-sleep"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-native-hw"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-hello-world"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-shopcart"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-file-hashing"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-classify"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-httprequest"
JV_GV_BENCHMARKS="$JV_GV_BENCHMARKS java/gv-video-processing"

JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS java/cr-file-hashing"
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS java/cr-hello-world"
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS java/cr-httprequest"
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS java/cr-classify"
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS java/cr-sleep"
JV_CR_BENCHMARKS="$JV_CR_BENCHMARKS java/cr-video-processing"

PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-sleep"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-hello-world"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-mst"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-bfs"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-pagerank"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-dna"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-classify"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-dynamic-html"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-compression"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-thumbnail"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-video-processing"
PY_GV_BENCHMARKS="$PY_GV_BENCHMARKS python/gv-uploader"

PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-thumbnail"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-compression"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-hello-world"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-dynamic-html"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-mst"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-uploader"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-sleep"
PY_CR_BENCHMARKS="$PY_CR_BENCHMARKS python/cr-video-processing"

JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS javascript/gv-sleep"
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS javascript/gv-hello-world"
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS javascript/gv-dynamic-html"
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS javascript/gv-thumbnail"
JS_GV_BENCHMARKS="$JS_GV_BENCHMARKS javascript/gv-uploader"

JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS javascript/cr-thumbnail"
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS javascript/cr-hello-world"
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS javascript/cr-dynamic-html"
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS javascript/cr-uploader"
JS_CR_BENCHMARKS="$JS_CR_BENCHMARKS javascript/cr-sleep"

if [ -z "$ARGO_HOME" ]
then
        echo "Please set ARGO_HOME first. It should point to a checkout of github.com/graalvm/argo."
        exit 1
fi

if [ -z "$JAVA_HOME" ]
then
        echo "Please set JAVA_HOME first. It should be a GraalVM with native-image available."
        exit 1
fi

BUILD_SET=""
read -p "Build Graalvisor's Java benchmarks (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    BUILD_SET="$BUILD_SET $JV_GV_BENCHMARKS"
fi
read -p "Build Graalvisor's Python benchmarks (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    BUILD_SET="$BUILD_SET $PY_GV_BENCHMARKS"
fi
read -p "Build Graalvisor's JavaScript benchmarks (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    BUILD_SET="$BUILD_SET $JS_GV_BENCHMARKS"
fi
read -p "Build OpenWhisk's Java benchmarks (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    BUILD_SET="$BUILD_SET $JV_CR_BENCHMARKS"
fi
read -p "Build OpenWhisk's Python benchmarks (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    BUILD_SET="$BUILD_SET $PY_CR_BENCHMARKS"
fi
read -p "Build OpenWhisk's JavaScript benchmarks (y or Y, everything else as no)? " -n 1 -r
echo    # move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    BUILD_SET="$BUILD_SET $JS_CR_BENCHMARKS"
fi

rm $LOG &> /dev/null
for benchmark in $BUILD_SET
do
    build_script=$DIR/../src/$benchmark/build_script.sh
    if [ -f "$build_script" ]; then
        echo -n "Building $benchmark..."
        if [[ $benchmark == *"/gv-"* ]]; then
            $DIR/build_benchmark.sh $build_script build_ni_sharedlibrary 2>&1 | tee -a $LOG | grep -q "Finished generating" || echo "Failed to build $benchmark!"
	else
            $DIR/build_benchmark.sh $build_script 2>&1 >> $LOG
        fi
        echo "done!"
    else
        echo "$build_script/build_script.sh DOES NOT exist, skipping."
    fi
done
