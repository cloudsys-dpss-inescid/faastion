#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor

JAVA_AGENT="$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar"

JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"
ERIM_INCLUDE="-I$ERIM_HOME/src/erim -I$ERIM_HOME/src/common"

CFLAGS="-Wall -g -fPIC -shared $JNI_INCLUDE"
CFLAGS_PROC="-Wall -g -fPIC $JNI_INCLUDE"
SFLAGS="$CFLAGS -O0 -fno-inline -I$GRAALVISOR_HOME/src/main/c/pkru-sandbox/src"

BENCHMARK_NAME="thumbnail"
SNIPPETS_DIR="$DIR/build/snippets"

CURRENT_LIBRARY_PATH=$LD_LIBRARY_PATH

function run_hotspot {
	rm -rf config-dir

    export LD_LIBRARY_PATH=$ARGO_HOME/graalvisor/build/libs:libs:$LD_LIBRARY_PATH
	$JAVA_HOME/bin/java \
		-Djava.awt.headless=true \
		-agentlib:native-image-agent=config-output-dir=config-dir/ \
		-cp build/libs/thumbnail-1.0-all.jar \
        -Djava.library.path=$LD_LIBRARY_PATH \
        com.thumbnail.Thumbnail
}

function build_native_binary {
	NI_BIN_OPTS="com.thumbnail.Thumbnail"
	cd build

	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
			--no-fallback \
			--enable-url-protocols=http \
			-cp $CLASS_PATH:libs/thumbnail-1.0-all.jar \
			-Djava.library.path=$LD_LIBRARY_PATH \
			-H:ConfigurationFileDirectories=../ni-agent-config,../config-dir \
			-H:+ReportExceptionStackTraces \
			$NI_BIN_OPTS \
			-H:Name=$GRAALVISOR_HOME/build/libs/$BENCHMARK_NAME-proc

	cd -
}

function build_ni {
	cd build

	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		--enable-url-protocols=http \
		-cp libs/thumbnail-1.0-all.jar:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
		-DGraalVisorGuest=true \
		-Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
		--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
		-H:ConfigurationFileDirectories=../ni-agent-config,../config-dir \
		-H:+ReportExceptionStackTraces \
		$NI_BIN_OPTS \
		-H:Name=lib$FUNCTION_ID

	cd -
}

function build_vanila_image {
	NI_BIN_OPTS="--shared"
	CLASS_PATH="$DIR/java/main"
	FUNCTION_ID="$BENCHMARK_NAME"

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
# bash $ARGO_HOME/graalvisor-lib/build.sh

# Move into the script directory.
cd $DIR &> /dev/null

# Build.
./gradlew clean shadowJar assemble

if [ ! -d config-dir ]; then
	echo
	read -p "Generate config-dir (run hotspot)? [y/n]" -s -n 1 -r
	while true; do
    	if [[ $REPLY =~ ^[Yy]$ ]] || [[ $REPLY =~ ^[Nn]$ ]]; then
			echo
			break
		fi
		read -s -n 1 -r
	done
	if [[ $REPLY =~ ^[Yy]$ ]]; then
		run_hotspot
	fi
fi

build_native_binary

build_vanila_image
