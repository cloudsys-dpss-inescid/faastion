#!/bin/bash

DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)

export ARGO_HOME="$DIR/.."
export RESOURCES_DIR="$ARGO_HOME/resources"
export WEBSERVER_IP=127.0.0.1 # change me
