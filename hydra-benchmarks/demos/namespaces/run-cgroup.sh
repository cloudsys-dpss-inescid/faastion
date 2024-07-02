#!/bin/bash

if [ -z "$1" ]
then
	echo "No argument supplied."
	exit 1
else
	CGROUP=$1
fi

# Create cgroup
start=$(date +%s%N)
mkdir /sys/fs/cgroup/$CGROUP
end=$(date +%s%N)
echo "Cgroup creation took $((($end - $start)/1000)) us"

# Size cgroup in terms of cpu
start=$(date +%s%N)
echo "12500 100000" >> /sys/fs/cgroup/$CGROUP/cpu.max # .125 core
end=$(date +%s%N)
echo "Cgroup setup took $((($end - $start)/1000)) us"

# Launch benchmark
sleep 1 &

# Add benchmark to cgroup
start=$(date +%s%N)
echo $! >> /sys/fs/cgroup/$CGROUP/cgroup.procs
end=$(date +%s%N)
echo "Adding process to cgroup took $((($end - $start)/1000)) us"

# Wait for the benchmark to finish
wait

# Remove cgroup
start=$(date +%s%N)
rmdir /sys/fs/cgroup/$CGROUP
end=$(date +%s%N)
echo "Removing cgroup took $((($end - $start)/1000)) us"
