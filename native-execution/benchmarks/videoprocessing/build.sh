#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

./gradlew clean shadowJar

export class_path=$(DIR)/build/libs/videoprocessing-1.0-all.jar
export entrypoint="com.videoprocessing.VideoProcessing"
