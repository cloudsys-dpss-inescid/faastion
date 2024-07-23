#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

# GCC
CC=gcc
# Musl GCC
export PATH=$ARGO_HOME/resources/x86_64-linux-musl-native/bin:$PATH
CC=x86_64-linux-musl-cc
LIBC_OPTION="--libc=musl"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor
JAVASSIST_HOME=$ARGO_HOME/native-execution/javassist

JAVA_AGENT="$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar"
JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"

CFLAGS="-Wall -g -fPIC $JNI_INCLUDE"
SFLAGS="$-O0 -fno-inline -I$ARGO_HOME/musl/include -I$GRAALVISOR_HOME/src/main/c/memisolation/src"

BENCHMARK_NAME="nativehw"
SNIPPETS_DIR="$DIR/build/snippets"

function build_ni {
	CLASS_PATH="classes/java/main"

	cd build
	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$LD_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
			--no-fallback \
			-cp $CLASS_PATH:libs/native-hw-1.0-all.jar:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
			-DGraalVisorGuest=true \
			-Djava.library.path=$LD_LIBRARY_PATH \
			-Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
			--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
			$LIBC_OPTION \
			-H:ConfigurationFileDirectories=../ni-agent-config \
			-H:+ReportExceptionStackTraces \
			$NI_BIN_OPTS \
			-H:Name=lib$BENCHMARK_NAME
}

function build_ni_sharedlibrary {
	NI_BIN_OPTS="--shared"
	build_ni
}

function build_java_agent {
	bash $JAVASSIST_HOME/build.sh
}

function build_native_library {
	$CC -static $CFLAGS -shared -o $GRAALVISOR_HOME/build/libs/lib$BENCHMARK_NAME-jni.so $DIR/src/main/c/HelloJNI.c
}

function build_snippets {
	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$LD_LIBRARY_PATH
	for file in "$SNIPPETS_DIR"/*.c; do
		name=$(basename "$file" .c)
		# LPI
		$CC $CFLAGS $SFLAGS -o $GRAALVISOR_HOME/build/libs/$name-proc $file -L$GRAALVISOR_HOME/build/libs -lmemiso -Wl,-rpath,$GRAALVISOR_HOME/build/libs
		# Actual snippet
		$CC $CFLAGS $SFLAGS -shared -o $GRAALVISOR_HOME/build/libs/lib$name.so $file -L$GRAALVISOR_HOME/build/libs -lmemiso
	done
}

function manipulate_bytecode {
	CLASS_PATH="build/classes/java/main"
	ENTRYPOINT="com.jni.HelloJNI"
	TOOL="NativeRedirection"
	
	mkdir -p $DIR/build/snippets
	
	export BENCHMARK_NAME="$BENCHMARK_NAME"
	export SNIPPETS_DIR="$SNIPPETS_DIR"
	export ENV="memisolation"
	$DEF_JAVA_HOME/bin/java \
			-cp $CLASS_PATH \
			-javaagent:$JAVA_AGENT=$TOOL::$CLASS_PATH \
			$ENTRYPOINT
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

# Build java agent.
build_java_agent

# Move into the script directory.
cd $DIR &> /dev/null

# Build application.
./gradlew clean shadowJar assemble

# Build native lib.
build_native_library

# Manipulate app's bytecode.
manipulate_bytecode

# Build generated snippets.
build_snippets

TARGET=$1
if [ ! -z "$TARGET" ]
then
	$TARGET
exit 0
else
	build_ni_sharedlibrary
	exit 0
fi
