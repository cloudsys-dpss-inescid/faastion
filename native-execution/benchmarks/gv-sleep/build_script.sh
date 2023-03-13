#!/bin/bash

function DIR {
	echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

function build_graalvisor_app {
	cd build
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		-cp libs/sleep-1.0-all.jar\
		-DGraalVisorGuest=true \
		-Dcom.oracle.svm.graalvisor.libraryPath=resources/main/com.oracle.svm.graalvisor.headers \
		--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
		-H:ConfigurationFileDirectories=../ni-agent-config \
		-H:+ReportExceptionStackTraces \
		--shared \
		-H:Name=libsleep
}

./gradlew clean shadowJar assemble

build_graalvisor_app

echo BENCHMARK_PATH=$(DIR)/build/libs/sleep-1.0.jar
