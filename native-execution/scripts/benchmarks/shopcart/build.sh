#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

mvn -Dmaven.test.skip=true package

export class_path=$(DIR)/target/shopcart-0.3.6.jar
export entrypoint="micronaut.benchmark.shopcart.Benchmark"
