#!/bin/bash

NUM_THREADS=20
ITER=100

for i in $(seq 1 $NUM_THREADS)
do
  rm -f jvm-$i.out
  touch jvm-$i.out
done

for i in $(seq 1 $NUM_THREADS)
do
  for n in $(seq 1 $ITER)
  do
    LD_LIBRARY_PATH=$ARGO_HOME/core/shared/ $DEF_JAVA_HOME/bin/java -cp build/libs/bfs-1.0-all.jar com.jni.BFS >> jvm-$i.out
  done &
done

wait
echo Finished
