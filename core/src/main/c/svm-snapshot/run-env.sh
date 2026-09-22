#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

# You can add any default options here
UNSHARE_OPTS="--kill-child --map-root-user --keep-caps --mount-proc -f -p"
ARCH_OPTS="-R"

unshare $UNSHARE_OPTS setarch $ARCH_OPTS env LD_PRELOAD=$(DIR)/deps/dlmalloc/hydralloc.so G_SLICE=always-malloc "$@"
