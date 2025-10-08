#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor

JAVA_AGENT="$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar"

JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"
ERIM_INCLUDE="-I$ERIM_HOME/src/erim -I$ERIM_HOME/src/common"

CFLAGS="-Wall -g -fPIC -shared $JNI_INCLUDE"
CFLAGS_PROC="-Wall -g -fPIC $JNI_INCLUDE"
SFLAGS="$CFLAGS -O0 -fno-inline -I$GRAALVISOR_HOME/src/main/c/jni -I$GRAALVISOR_HOME/src/main/c/pkru-sandbox/src"

BENCHMARK_NAME="classify"
SNIPPETS_DIR="$DIR/build/snippets"

CURRENT_LIBRARY_PATH=$LD_LIBRARY_PATH

function run_hotspot {
	rm -rf config-dir

    export LD_LIBRARY_PATH=$ARGO_HOME/graalvisor/build/libs:libs:$LD_LIBRARY_PATH
	$JAVA_HOME/bin/java \
		-Djava.awt.headless=true \
		-agentlib:native-image-agent=config-output-dir=config-dir/ \
		-cp build/libs/classify-1.0-all.jar \
        -Djava.library.path=$LD_LIBRARY_PATH \
        com.classify.Classify
}

function build_native_binary {
	NI_BIN_OPTS="com.classify.Classify"
	cd build

	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
			--no-fallback \
			--enable-url-protocols=http \
			-Djava.awt.headless=true \
			-cp $CLASS_PATH:libs/classify-1.0-all.jar \
			-Djava.library.path=$LD_LIBRARY_PATH \
			-H:ConfigurationFileDirectories=../ni-agent-config,../config-dir \
			-H:+ReportExceptionStackTraces \
			$NI_BIN_OPTS \
			-H:Name=$GRAALVISOR_HOME/build/libs/$BENCHMARK_NAME-proc

	cd -
}

function build_ni {
	cd build/${FUNCTION_ID}

	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		--enable-url-protocols=http \
		-Djava.awt.headless=true \
		-cp $CLASS_PATH:../libs/classify-1.0-all.jar:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
		-DGraalVisorGuest=true \
		-Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
		--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
		-H:ConfigurationFileDirectories=../../ni-agent-config,../../config-dir \
		-H:+ReportExceptionStackTraces \
		$NI_BIN_OPTS \
		-H:Name=lib$FUNCTION_ID

	rm -rf /tmp/apps/${FUNCTION_ID} &> /dev/null
	zipfile=lib${FUNCTION_ID}.zip
	zip --junk-paths $zipfile *.so *.h
	cp $zipfile $RESOURCES_DIR/apps/.

	cd -
}

function build_faastion_image {
	FUNCTION_ID="$BENCHMARK_NAME"-plugin
	mkdir -p build/${FUNCTION_ID}
	manipulate_bytecode
	build_snippets

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

function build_snippets {
	make
}

function manipulate_bytecode {
	CLASS_PATH=$ARGO_HOME/native-execution/instrumentation/target/BytecodeTransformer-1.0-jar-with-dependencies.jar
	ENTRYPOINT=org.faastion.javassist.BytecodeTransformer

	rm -f $GRAALVISOR_HOME/shared/lib${FUNCTION_ID}-wrapper.so

	mkdir -p $DIR/build/snippets

	export BENCHMARK_NAME="$BENCHMARK_NAME"
	export SNIPPETS_DIR="$SNIPPETS_DIR"
	export FUNCTION_ID="$FUNCTION_ID"
	export ENV="memisolation"

	$DEF_JAVA_HOME/bin/java -cp $CLASS_PATH $ENTRYPOINT build/libs/classify-1.0-all.jar

	echo "check snippets"
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
fi

# Build graalvisor lib.
#bash $ARGO_HOME/graalvisor-lib/build.sh

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

#build_native_binary

build_vanila_image

build_faastion_image
