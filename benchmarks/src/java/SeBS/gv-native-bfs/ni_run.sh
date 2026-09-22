#!/bin/bash

NUM_THREADS=20
ITER=1000

for i in $(seq 1 $NUM_THREADS)
do
  rm -f ni-$i.out
  touch ni-$i.out
done

for i in $(seq 1 $NUM_THREADS)
do
    for n in $(seq 1 $ITER)
    do
	LD_LIBRARY_PATH=$ARGO_HOME/core/shared/ ./build/bfs-proc >> ni-$i.out
    done &
done

wait
echo Finished
