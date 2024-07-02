#!/bin/bash

if [ -z "$JAVA_HOME" ]
then
        echo "JAVA_HOME is not set. Recommended JVM: graalvm-ee-java11-22.1.0"
fi

function build_server {
	cd build
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		-cp libs/demo-sharedmemory-1.0-all.jar \
		-H:+ReportExceptionStackTraces \
		-H:Name=sharedmemoryserver \
                com.demo.sharedmemory.SharedMemoryServer
	cd -
}

function build_client {
	cd build
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		-cp libs/demo-sharedmemory-1.0-all.jar \
		-H:+ReportExceptionStackTraces \
		-H:Name=sharedmemoryclient \
                com.demo.sharedmemory.SharedMemoryClient
	cd -
}

./gradlew clean shadowJar assemble

build_server
build_client
