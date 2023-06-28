#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

mvn package -DskipTests

export class_path=$(DIR)/target/petclinic-jpa-0.1.6.jar
export entrypoint="org.springframework.boot.loader.JarLauncher"
