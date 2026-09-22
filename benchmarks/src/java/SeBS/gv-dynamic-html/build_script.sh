#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

CORE_DIR=$ARGO_HOME/core

JAVA_AGENT="$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar"

JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"
ERIM_INCLUDE="-I$ERIM_HOME/src/erim -I$ERIM_HOME/src/common"

CFLAGS="-Wall -g -fPIC -shared $JNI_INCLUDE"
CFLAGS_PROC="-Wall -g -fPIC $JNI_INCLUDE"
SFLAGS="$CFLAGS -O0 -fno-inline -I$CORE_DIR/src/main/c/pkru-sandbox/src"

BENCHMARK_NAME="dynamic-html"
SNIPPETS_DIR="$DIR/build/snippets"

CURRENT_LIBRARY_PATH=$LD_LIBRARY_PATH

function build_native_binary {
	NI_BIN_OPTS="com.dynamic_html.DynamicHTML"
	cd build

	export LD_LIBRARY_PATH=$CORE_DIR/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
			--no-fallback \
			--enable-url-protocols=http \
			-cp $CLASS_PATH:libs/dynamic-html-1.0-all.jar \
			-Djava.library.path=$LD_LIBRARY_PATH \
			-H:ConfigurationFileDirectories=../ni-agent-config \
			-H:+ReportExceptionStackTraces \
			$NI_BIN_OPTS \
			-H:Name=$CORE_DIR/build/libs/$BENCHMARK_NAME-proc

	cd -
}

function build_ni {
	cd build/${FUNCTION_ID}

	export LD_LIBRARY_PATH=$CORE_DIR/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		--enable-url-protocols=http \
		-cp ../libs/dynamic-html-1.0-all.jar:$ARGO_HOME/common/build/libs/faastion-lib-1.0-guest.jar \
		-DGraalVisorGuest=true \
		-Dcom.oracle.svm.faastion.libraryPath=$ARGO_HOME/common/build/resources/main/com.oracle.svm.faastion.headers \
		--initialize-at-run-time=com.oracle.svm.faastion.utils.JsonUtils \
		-H:ConfigurationFileDirectories=../../ni-agent-config \
		-H:+ReportExceptionStackTraces \
		$NI_BIN_OPTS \
		-H:Name=lib$FUNCTION_ID

	rm -rf /tmp/apps/lib${FUNCTION_ID} &> /dev/null
	zipfile=lib${FUNCTION_ID}.zip
	rm -f $RESOURCES_DIR/apps/$zipfile
	zip --junk-paths $zipfile *.so *.h
	cp $zipfile $RESOURCES_DIR/apps/.

	cd -
}

function build_faastion_image {
	FUNCTION_ID="$BENCHMARK_NAME"-plugin
	mkdir -p build/${FUNCTION_ID}
	
	NI_BIN_OPTS="--shared"
	CLASS_PATH="$DIR/output"
	build_ni
}

function build_vanila_image {
	FUNCTION_ID="$BENCHMARK_NAME"
	mkdir -p build/${FUNCTION_ID}

	NI_BIN_OPTS="--shared"
	CLASS_PATH="$DIR/java/main"
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

if [ -z "$RESOURCES_DIR" ]
then
	echo "Please set RESOURCES_DIR first."
	exit 1
else
	mkdir -p $RESOURCES_DIR/apps
fi

# Move into the script directory.
cd $DIR &> /dev/null

# Build.
./gradlew clean shadowJar assemble

#build_native_binary

build_vanila_image

build_faastion_image
