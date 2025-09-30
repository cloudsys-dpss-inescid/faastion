#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor

JAVA_AGENT="$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar"

JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"
ERIM_INCLUDE="-I$ERIM_HOME/src/erim -I$ERIM_HOME/src/common"

CFLAGS="-Wall -g -fPIC -shared $JNI_INCLUDE"
CFLAGS_PROC="-Wall -g -fPIC $JNI_INCLUDE"
SFLAGS="$CFLAGS -O0 -fno-inline -I$GRAALVISOR_HOME/src/main/c/pkru-sandbox -I$GRAALVISOR_HOME/src/main/c/dlmalloc"

BENCHMARK_NAME="pagerank"
SNIPPETS_DIR="$DIR/build/snippets"

CURRENT_LIBRARY_PATH=$LD_LIBRARY_PATH

function build_native_binary {
	NI_BIN_OPTS="com.jni.PageRank"
	cd build

	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
			--no-fallback \
			-cp $CLASS_PATH:libs/pagerank-1.0-all.jar \
			-Djava.library.path=$LD_LIBRARY_PATH \
			-H:ConfigurationFileDirectories=../ni-agent-config \
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
			-cp $CLASS_PATH:libs/pagerank-1.0-all.jar:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
			-DGraalVisorGuest=true \
			-Djava.library.path=$LD_LIBRARY_PATH \
			-Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
			--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
			-H:ConfigurationFileDirectories=../ni-agent-config \
			-H:+ReportExceptionStackTraces \
			$NI_BIN_OPTS \
			-H:Name=lib$FUNCTION_ID

	cd -
}

function build_faastion_image {
	FUNCTION_ID="$BENCHMARK_NAME"
	manipulate_bytecode
	build_snippets

	NI_BIN_OPTS="--shared"
	CLASS_PATH="$DIR/output"
	build_ni
}

function build_vanila_image {
	NI_BIN_OPTS="--shared"
	CLASS_PATH="$DIR/java/main"
	FUNCTION_ID="$BENCHMARK_NAME"_vanilla

	build_ni
}

function build_java_agent {
	bash $JAVASSIST_HOME/build.sh
}

function build_native_library {
	gcc $CFLAGS -Wl,--no-undefined -I"${IGRAPH_HOME}/include/igraph" -L"${IGRAPH_HOME}/lib" -o $GRAALVISOR_HOME/build/libs/lib$BENCHMARK_NAME-jni.so $DIR/src/main/c/PageRank.c -ligraph -lm -fopenmp -lstdc++
}

function build_snippets {
	make
}

function manipulate_bytecode {
	CLASS_PATH=$ARGO_HOME/native-execution/instrumentation/target/BytecodeTransformer-1.0-jar-with-dependencies.jar
	ENTRYPOINT=org.faastion.javassist.BytecodeTransformer

	rm -f $GRAALVISOR_HOME/build/libs/lib${FUNCTION_ID}-wrapper.so

	mkdir -p $DIR/build/snippets

	export BENCHMARK_NAME="$BENCHMARK_NAME"
	export SNIPPETS_DIR="$SNIPPETS_DIR"
	export FUNCTION_ID="$FUNCTION_ID"
	export ENV="memisolation"

	$DEF_JAVA_HOME/bin/java -cp $CLASS_PATH $ENTRYPOINT build/libs/pagerank-1.0-all.jar

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

if [ -z "$JAVASSIST_HOME" ]
then
	echo "Please set JAVASSIST_HOME first."
	exit 1
fi

# Build graalvisor lib.
# bash $ARGO_HOME/graalvisor-lib/build.sh

# Build java agent.
# build_java_agent

# Move into the script directory.
cd $DIR &> /dev/null

# Build application.
./gradlew clean shadowJar assemble

#build_native_binary # for LPI

build_native_library # compile jni code

build_vanila_image # to benchmark vanila

build_faastion_image # to benchmark faastion

exit 0
