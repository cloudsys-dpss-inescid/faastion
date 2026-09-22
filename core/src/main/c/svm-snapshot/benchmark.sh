#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

CONC=1
ITERS=1

# Initial setup.
#mkdir $(DIR)/tmpapp
#cp $ARGO_HOME/benchmarks/data/apps/gv-js-thumbnail.zip $(DIR)/tmpapp
#cd $(DIR)/tmpapp
#unzip gv-js-thumbnail.zip
#cd -

#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-js-dynamic-html.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-js-hello-world.so"
export BENCHMARKS="$BENCHMARKS $ARGO_HOME/graalvisor/src/main/c/svm-snapshot/tmpapp/gv-js-thumbnail.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-js-uploader.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-jv-file-hashing.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-jv-hello-world.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-jv-httprequest.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-bfs.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-compression.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-dna.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-dynamic-html.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-hello-world.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-mst.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-pagerank.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-thumbnail.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-uploader.so"

# Disabled (sub-processes).
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-jv-classify.zip"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-py-video-processing.so"
#export BENCHMARKS="$BENCHMARKS $ARGO_HOME/benchmarks/data/apps/gv-jv-video-processing.so"

mkdir $(DIR)/results &> /dev/null
for benchmark in $BENCHMARKS
do
    bdir=$(DIR)/results/$(basename $benchmark)
    mkdir $bdir &> /dev/null
    rm -f snapshot.mem
    rm -f snapshot.meta
    $(DIR)/run-env.sh /bin/time -v $(DIR)/main normal     $benchmark $CONC $ITERS 2>&1 | tee $bdir/normal.log
    $(DIR)/run-env.sh /bin/time -v $(DIR)/main checkpoint $benchmark $CONC $ITERS 2>&1 | tee $bdir/checkpoint.log
    $(DIR)/run-env.sh /bin/time -v $(DIR)/main restore    $benchmark $CONC $ITERS 2>&1 | tee $bdir/restore.log
done
