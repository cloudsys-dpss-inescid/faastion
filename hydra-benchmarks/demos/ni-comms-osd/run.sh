#!/bin/bash

# run.sh [warmup_runs] [runs] [bufsize] [log_dir_name]

if [ ! -f build/libs/demo-ni-osdcomms-1.0-all.jar ]; then
	echo "demoosdcomms JAR missing, rebuilding"
	./build.sh
elif [ ! -f build/demoosdcomms ]; then
	echo "demoosdcomms native image missing, rebuilding"
	./build.sh
fi

. ./.env
echo "JAVA_HOME=$JAVA_HOME"
echo "GRAALVM_HOME=$GRAALVM_HOME"

runs=1000
warmup=1000000
bufsize=0
timestamp=$(date +%s)
if [ $# -ge 1 ]; then
	warmup=$1
	if [ $# -ge 2 ]; then
		runs=$2
		if [ $# -ge 3 ]; then
			bufsize=$3
			if [ $# -ge 4 ]; then
				timestamp=$4
			fi
		fi
	fi
fi

cd build

store_dir=../logs/$timestamp
mkdir -p $store_dir

tests=(
	# gson
	jackson
	# kryo
)

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

client_profiles_path=../profiles/client_profiles.iprof
server_profiles_path=../profiles/server_profiles.iprof
test -f $client_profiles_path && test -f $server_profiles_path
profiles_exist=$?

set -e;

for test in "${tests[@]}"; do
	mkdir -p $store_dir/$test

	echo "Running $test on JVM with $warmup warmup runs and $runs iterations"

	$JAVA_HOME/bin/java \
		-ea \
		-jar libs/demo-ni-osdcomms-1.0-all.jar \
		--server $test --runs $runs --warmup $warmup --bufsize $bufsize \
		> $store_dir/$test/jvm_server.log 2>&1 &

	sleep 1

	$JAVA_HOME/bin/java \
		-ea \
		-jar libs/demo-ni-osdcomms-1.0-all.jar \
		--client $test --runs $runs --warmup $warmup --bufsize $bufsize \
		> $store_dir/$test/jvm_client.log 2>&1 &

	wait

	echo "Running $test on SVM with $warmup warmup runs and $runs iterations"

	if [ $profiles_exist -eq 0 ]; then
		echo "Using profiles from previous runs..."

		./demoosdcomms \
			--server $test --runs $runs --warmup $warmup --bufsize $bufsize \
			> $store_dir/$test/svm_server.log 2>&1 &

		sleep 1

		./demoosdcomms \
			--client $test --runs $runs --warmup $warmup --bufsize $bufsize \
			> $store_dir/$test/svm_client.log 2>&1 &
	else
		echo "Generating profiles in this run... RE-RUN FOR RESULTS WITH PGO!!!"
		
		./demoosdcomms \
			-XX:ProfilesDumpFile=$client_profiles_path \
			--server $test --runs $runs --warmup $warmup --bufsize $bufsize \
			> $store_dir/$test/svm_server.log 2>&1 &

		sleep 1

		./demoosdcomms \
			-XX:ProfilesDumpFile=$server_profiles_path \
			--client $test --runs $runs --warmup $warmup --bufsize $bufsize \
			> $store_dir/$test/svm_client.log 2>&1 &
	fi

	wait
done
