#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

JAVA_AGENT="$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar"

JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"
ERIM_INCLUDE="-I$ERIM_HOME/src/erim -I$ERIM_HOME/src/common"

CFLAGS="-Wall -g -fPIC -shared $JNI_INCLUDE"
SFLAGS="$CFLAGS -O0 -fno-inline $ERIM_INCLUDE -I$ARGO_HOME/graalvisor/src/main/c/memisolation/src -DSNI_DBG"

BENCHMARK_NAME="aes"
SNIPPETS_DIR="$DIR/build/snippets"

function run_hotspot {
	rm -rf config-dir
        CLASS_PATH="classes/java/main"

        cd build
        export LD_LIBRARY_PATH=$ARGO_HOME/graalvisor/build/libs:libs:$LD_LIBRARY_PATH
	$JAVA_HOME/bin/java \
		-Djava.awt.headless=true \
		-agentlib:native-image-agent=config-output-dir=config-dir/ \
		-cp $CLASS_PATH:libs/aes-1.0-all.jar \
                -Djava.library.path=$LD_LIBRARY_PATH \
                com.jni.AESEncryption
}

function build_ni {
        CLASS_PATH="classes/java/main"

	cd build
        export LD_LIBRARY_PATH=libs:$LD_LIBRARY_PATH
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		-cp $CLASS_PATH:libs/aes-1.0-all.jar:$ARGO_HOME/graalvisor-lib/build/libs/graalvisor-lib-1.0-guest.jar \
		-DGraalVisorGuest=true \
                -Djava.library.path=$LD_LIBRARY_PATH \
		-Dcom.oracle.svm.graalvisor.libraryPath=$ARGO_HOME/graalvisor-lib/build/resources/main/com.oracle.svm.graalvisor.headers \
		--initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
		-H:ConfigurationFileDirectories=../ni-agent-config \
		-H:+ReportExceptionStackTraces \
		$NI_BIN_OPTS \
		-H:Name=lib$BENCHMARK_NAME
}

function build_ni_standalone {
	NI_BIN_OPTS="com.jni.AESEncryption"
	build_ni
}

function build_ni_sharedlibrary {
	NI_BIN_OPTS="--shared"
	build_ni
}

function build_native_library {
    gcc $CFLAGS -o $ARGO_HOME/graalvisor/build/libs/lib$BENCHMARK_NAME-jni.so $DIR/src/main/c/AESEncryption.c
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

# Move into the script directory.
cd $DIR &> /dev/null

# Build application.
./gradlew clean shadowJar assemble

# Build native lib.
build_native_library

TARGET=$1
if [ ! -z "$TARGET" ]
then
        $TARGET
	exit 0
else
        build_ni_sharedlibrary
        exit 0
fi
