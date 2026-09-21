#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

if [ -z "$ARGO_HOME" ]; then
	echo "Please set ARGO_HOME first."
	exit 1
fi

if [ -z "$RESOURCES_DIR" ]; then
	echo "Please set RESOURCES_DIR first."
	exit 1
fi

docker run --rm -v $ARGO_HOME/graalvisor/shared:/faastion/graalvisor/shared \
	-v $ARGO_HOME/benchmarks/src/java/SeBS:/faastion/benchmarks/src/java/SeBS \
	-v $RESOURCES_DIR:/resources \
	-w /faastion/benchmarks/ \
	--network host \
	-it --entrypoint bash faastion
