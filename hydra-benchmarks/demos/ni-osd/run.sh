#!/bin/bash

# run.sh [warmup_runs] [runs] [log_dir_name]

if [ ! -f build/libs/demo-ni-osd-1.0-all.jar ]; then
	echo "demoosd JAR missing, rebuilding"
	./build.sh
elif [ ! -f build/demoosd ]; then
	echo "demoosd native image missing, rebuilding"
	./build.sh
fi

. ./.env

echo "JAVA_HOME=$JAVA_HOME"
echo "GRAALVM_HOME=$GRAALVM_HOME"

runs=1000
warmup=1000000
timestamp=$(date +%s)
if [ $# -ge 1 ]; then
	warmup=$1
	if [ $# -ge 2 ]; then
		runs=$2
		if [ $# -ge 3 ]; then
			timestamp=$3
		fi
	fi
fi

cd build

store_dir=../logs/$timestamp
mkdir -p $store_dir

tests=(
	sRInt4
	sRInt8
	sRInt32
	sRStr064
	sRStr128
	sRStr256
	sALst4
	sALst8
	sALst32
	sALst64
	sBigObj
	sHMap4
	sHMap8
	sHMap32
	sHMap64
	dRInt4
	dRInt8
	dRInt32
	dRStr064
	dRStr128
	dRStr256
	dALst4
	dALst8
	dALst32
	dALst64
	dHMap4
	dHMap8
	dHMap32
	dHMap64
	dBigObj
)

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

profiles_path=../profiles/profiles.iprof
test -f $profiles_path
profiles_exist=$?

set -e;

for test in "${tests[@]}"; do
	mkdir -p $store_dir/$test

	echo "Running $test on JVM with $warmup warmup runs and $runs iterations"

	$JAVA_HOME/bin/java \
		-ea \
		-Xmx16g \
		-jar libs/demo-ni-osd-1.0-all.jar \
		$test --runs $runs --warmup $warmup \
		> $store_dir/$test/"$test"_jvm.log 2>&1 &
	wait

	echo "Running $test on SVM with $warmup warmup runs and $runs iterations"

	if [ $profiles_exist -eq 0 ]; then
		echo "Using profiles from previous runs..."
		./demoosd \
			$test --runs $runs --warmup $warmup \
			> $store_dir/$test/"$test"_svm.log 2>&1 &
	else
		echo "Generating profiles in this run... RE-RUN FOR RESULTS WITH PGO!!!"
		./demoosd \
			-XX:ProfilesDumpFile=$profiles_path \
			$test --runs $runs --warmup $warmup \
			> $store_dir/$test/"$test"_svm.log 2>&1 &
	fi

	wait
done
