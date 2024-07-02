#!/bin/bash

if [[ -z "${ARGO_HOME}" ]]; then
	echo "ARGO_HOME is not defined. Existing..."
	exit 1
fi

if [[ -z "${JAVA_HOME}" ]]; then
	echo "JAVA_HOME is not defined. Existing..."
	exit 1
fi

function build_graalvisor_host {
	cd build
	$JAVA_HOME/bin/native-image \
		-H:+ReportExceptionStackTraces \
		--no-fallback \
		-cp libs/virtualization-benchmarks-1.0-all.jar \
		-DGraalVisorHost \
		-Dcom.oracle.svm.graalvisor.libraryPath=resources/main/com.oracle.svm.graalvisor.headers \
		--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
                GraalvisorHostIsolateScalability \
                graalvisorhost
}

./gradlew clean shadowJar assemble

build_graalvisor_host
