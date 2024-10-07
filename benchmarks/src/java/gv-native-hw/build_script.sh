#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

GRAALVISOR_HOME=$ARGO_HOME/graalvisor

JAVA_AGENT="$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar"

JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"
ERIM_INCLUDE="-I$ERIM_HOME/src/erim -I$ERIM_HOME/src/common"

CFLAGS="-Wall -g -fPIC -shared $JNI_INCLUDE"
CFLAGS_PROC="-Wall -g -fPIC $JNI_INCLUDE"
# SFLAGS="$CFLAGS -O0 -fno-inline $ERIM_INCLUDE -I$GRAALVISOR_HOME/src/main/c/memisolation/src"
# SFLAGS_PROC="$CFLAGS_PROC -O0 -fno-inline $ERIM_INCLUDE -I$GRAALVISOR_HOME/src/main/c/memisolation/src"
SFLAGS="$CFLAGS -O0 -fno-inline -I$GRAALVISOR_HOME/src/main/c/pkru-sandbox/src"

SNIPPETS_DIR="$DIR/build/snippets"

CURRENT_LIBRARY_PATH=$LD_LIBRARY_PATH

function build_ni {
	CLASS_PATH="$DIR/output"

	cd build

	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$CURRENT_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
			--no-fallback \
			-cp $CLASS_PATH:libs/native-hw-1.0-all.jar:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
			-DGraalVisorGuest=true \
			-Djava.library.path=$LD_LIBRARY_PATH \
			-Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
			--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
			-H:ConfigurationFileDirectories=../ni-agent-config \
			-H:+ReportExceptionStackTraces \
			$NI_BIN_OPTS \
			-H:Name=lib$BENCHMARK_REALNAME
	
	cd -
}

function build_ni_sharedlibrary {
	NI_BIN_OPTS="--shared"
	build_ni
}

function build_java_agent {
	bash $JAVASSIST_HOME/build.sh
}

function build_native_library {
	# musl-gcc -static $CFLAGS -o $GRAALVISOR_HOME/build/libs/lib$BENCHMARK_NAME-jni.so $DIR/src/main/c/HelloJNI.c
	gcc --shared -fpic $CFLAGS -o $GRAALVISOR_HOME/build/libs/lib$BENCHMARK_REALNAME-jni.so $DIR/src/main/c/HelloJNI.c
}

# TODO
function build_native_exec {
	export LD_LIBRARY_PATH=$GRAALVISOR_HOME/build/libs:libs:$CURRENT_LIBRARY_PATH
	for file in "$SNIPPETS_DIR"/*.c; do
		name=$(basename "$file" .c)
		gcc $SFLAGS_PROC -o $GRAALVISOR_HOME/build/libs/$name-proc $file -L$GRAALVISOR_HOME/build/libs -lmemiso -Wl,-rpath,$GRAALVISOR_HOME/build/libs
	done
}

function build_snippets {
	pathname=$(ls "$SNIPPETS_DIR"/*.c)
	file=${pathname##*/}
	name=${file%.*}
	gcc $SFLAGS -o $GRAALVISOR_HOME/build/libs/lib${BENCHMARK_REALNAME}-${name}.so $pathname -L$GRAALVISOR_HOME/build/libs -lpkru
}

function manipulate_bytecode {
	CLASS_PATH="build/classes/java/main"
	ENTRYPOINT="com.jni.HelloJNI"
	TOOL="JNITemplateBuilder"

	rm -f $GRAALVISOR_HOME/build/libs/lib${BENCHMARK_REALNAME}-printHello.so

	mkdir -p $DIR/build/snippets
	
	export BENCHMARK_NAME="$BENCHMARK_REALNAME"
	export SNIPPETS_DIR="$SNIPPETS_DIR"
	export ENV="memisolation"

	# cmd="$DEF_JAVA_HOME/bin/java \
	# 		-cp $CLASS_PATH \
	# 		-javaagent:$JAVA_AGENT=$TOOL:com.jni:output \
	# 		$ENTRYPOINT"
	# echo "$cmd"

	$DEF_JAVA_HOME/bin/java \
			-cp $CLASS_PATH \
			-javaagent:$JAVA_AGENT=$TOOL::output \
			$ENTRYPOINT

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
bash $ARGO_HOME/graalvisor-lib/build.sh

# Build java agent.
build_java_agent

# Move into the script directory.
cd $DIR &> /dev/null

# Build application.
./gradlew clean shadowJar assemble

CONCURRENCY_LEVEL=32
for i in $(seq 1 $CONCURRENCY_LEVEL); do
	BENCHMARK_REALNAME="nativehw${i}"
	build_native_library
	manipulate_bytecode
	build_snippets
	build_ni_sharedlibrary
done
exit 0