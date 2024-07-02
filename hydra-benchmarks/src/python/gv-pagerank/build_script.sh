#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

function run_hotspot {
        $JAVA_HOME/bin/java \
		-Dcom.oracle.svm.graalvisor.polyglotengine.language=python \
		-Dcom.oracle.svm.graalvisor.polyglotengine.entrypoint=main \
		-Dcom.oracle.svm.graalvisor.polyglotengine.source=$DIR/src/main/python/main.py \
                -cp build/libs/pr-1.0-all.jar \
                com.pr.PageRank
}

function build_ni {
	cd build
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		--enable-url-protocols=http \
		-cp libs/pr-1.0-all.jar:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
		-DGraalVisorGuest=true \
		-Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
		-Dcom.oracle.svm.graalvisor.polyglotengine.language=python \
		-Dcom.oracle.svm.graalvisor.polyglotengine.entrypoint=main \
		-Dcom.oracle.svm.graalvisor.polyglotengine.source=$DIR/src/main/python/main.py \
		--initialize-at-build-time=com.pr.PageRank \
		--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
		-H:ConfigurationFileDirectories=../ni-agent-config \
		--language:python \
		-H:+ReportExceptionStackTraces \
		$NI_BIN_OPTS \
		-H:Name=libpr
}

function build_ni_standalone {
	NI_BIN_OPTS="com.pr.PageRank"
	build_ni
}

function build_ni_sharedlibrary {
	NI_BIN_OPTS="--shared"
	build_ni
}

if [ -z "$ARGO_HOME" ]
then
        echo "Please set ARGO_HOME first. It should point to a checkout of github.com/graalvm/argo."
        exit 1
fi

if [ -z "$JAVA_HOME" ]
then
        echo "Please set JAVA_HOME first. It should be a GraalVM with native-image available."
        exit 1
fi

# Build graalvisor lib.
bash $ARGO_HOME/graalvisor-lib/build.sh

# Move into the script directory.
cd $DIR &> /dev/null

# Build.
./gradlew clean shadowJar assemble

TARGET=$1
if [ ! -z "$TARGET" ]
then
        $TARGET
	exit 0
else
	read -p "Run benchmark on hotspot (y or Y, everything else as no)? " -n 1 -r
        echo # move to a new line
        if [[ $REPLY =~ ^[Yy]$ ]]
        then
                run_hotspot
                exit 0
        fi
	read -p "Build standalone Native Image (y or Y, everything else as no)? " -n 1 -r
        echo # move to a new line
        if [[ $REPLY =~ ^[Yy]$ ]]
        then
                build_ni_standalone
                exit 0
        fi
	read -p "Build shared library Native Image (y or Y, everything else as no)? " -n 1 -r
        echo # move to a new line
        if [[ $REPLY =~ ^[Yy]$ ]]
        then
                build_ni_sharedlibrary
                exit 0
        fi
fi
