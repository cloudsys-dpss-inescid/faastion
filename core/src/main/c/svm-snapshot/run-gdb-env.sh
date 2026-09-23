#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

# You can add any default options here
UNSHARE_OPTS="--kill-child --map-root-user --keep-caps --mount-proc -f -p"
ARCH_OPTS="-R"

unshare $UNSHARE_OPTS \
setarch $ARCH_OPTS \
gdb -x gdb-commands.txt -ex "set environment LD_PRELOAD $(DIR)/deps/dlmalloc/hydralloc.so" -ex "set environment G_SLICE always-malloc" --args $@
