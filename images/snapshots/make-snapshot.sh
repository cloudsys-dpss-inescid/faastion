#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

function make_snapshot {
    image_name=$1

    echo "($ID) Starting vm..."
    $DIR/start-vm.sh $ID $IP $image_name &> $DIR/$image_name/start-vm-$ID.log &
    sleep 1
    echo "($ID) Configuring vm..."
    $DIR/config-vm.sh $ID $IP $image_name &> $DIR/$image_name/config-vm-$ID.log

    if [ "$image_name" == "graalvisor" ] && [ -n "$FUNCTION_ENTRYPOINT" ] && [ -n "$FUNCTION_BINARY" ]; then
        response=$(curl -X POST "$IP":8080/register?name=function\&entryPoint="$FUNCTION_ENTRYPOINT"\&language=java\&sandbox=isolate -H 'Content-Type: application/json' --data-binary @"$FUNCTION_BINARY")
        echo $response  # To ensure that VM is up and the function is registered.
        # curl -X POST "$IP":8080 -H 'Content-Type: application/json' -d '{"name":"function","async":"false","arguments":"{\"memory\":\"128\",\"duration\":\"1\"}"}'  # Example invocation for gv-genericapp.
    fi

    echo "($ID) Snapshotting vm..."
    $DIR/snapshot-vm.sh $ID $IP $image_name &> $DIR/$image_name/snapshot-vm-$ID.log

    echo "($ID) Stopping vm..."
    $DIR/stop-vm.sh $ID $IP $image_name &> $DIR/$image_name/stop-vm-$ID.log
}

IMAGE=$1
FUNCTION_ENTRYPOINT=$2
FUNCTION_BINARY=$3

ID=3
IP=172.18.0.3

if [ -z "$IMAGE" ]; then
    for image_name in graalvisor hotspot hotspot-agent java-openwhisk; do
        make_snapshot $image_name
    done
else
    make_snapshot $IMAGE
fi
