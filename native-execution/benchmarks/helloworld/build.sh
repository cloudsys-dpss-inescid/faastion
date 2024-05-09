#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

./gradlew clean shadowJar

export class_path=$(DIR)/build/libs/hello-world-1.0-all.jar
export entrypoint="com.hello_world.HelloWorld"
