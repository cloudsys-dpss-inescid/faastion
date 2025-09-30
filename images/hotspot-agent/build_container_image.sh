#!/bin/bash

# This script creates Docker images that can be used to run HotSpot lambdas.
# Important note: make sure you have the "argo-hotspot" image built locally before running this script.

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_JAR=$DIR/../../graalvisor/build/libs/graalvisor-1.0-all.jar

docker image rm argo-hotspot-agent:latest

GRAALVISOR_JAR_FILENAME="$(basename -- $GRAALVISOR_JAR)"
GRAALVISOR_ENTRYPOINT="org.graalvm.argo.graalvisor.Main"
AGENT_OPTIONS="-agentlib:native-image-agent=config-merge-dir=config,caller-filter-file=caller-filter-config.json,config-write-initial-delay-secs=90,config-write-period-secs=60"
ENTRYPOINT_COMMAND="/jvm/bin/java -Djava.library.path=/jvm/lib $AGENT_OPTIONS -cp $GRAALVISOR_JAR_FILENAME $GRAALVISOR_ENTRYPOINT"

docker build \
    --build-arg ENTRYPOINT_COMMAND_AGENT="$ENTRYPOINT_COMMAND" \
    --tag=argo-hotspot-agent $DIR
