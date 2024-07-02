#!/bin/bash

# Note, we are using a Java 8 runtime so we need to compile with Java 8.
JAVA_HOME=/usr/lib/jvm/openjdk1.8.0_302-jvmci-20.3-b23
./gradlew -Dorg.gradle.java.home=$JAVA_HOME clean shadowJar assemble
body=$(base64 --wrap=0 build/libs/videoprocessing-1.0-all.jar)
echo "{ \"value\": { \"binary\": \"True\", \"main\": \"com.videoprocessing.VideoProcessing\", \"code\": \"$body\" } }" > init.json
