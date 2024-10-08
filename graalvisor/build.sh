#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
GRAALVISOR_HOME=$DIR/build/native-image
GRAALVISOR_JAR=$DIR/build/libs/graalvisor-1.0-all.jar

GREEN='\033[0;32m'
NC='\033[0m' # No Color

CC=gcc

function build_pkru_sandbox {
	release=$(uname -r)
	major_version=${release%%.*}
	release=${release#*.}
	minor_version=${release%%.*}

    JNI_INCLUDE="-I$DEF_JAVA_HOME/include -I$DEF_JAVA_HOME/include/linux"
	CFLAGS="-Wall -g -fno-inline -fPIC -shared"

	if [ $major_version -ge 5 ] && [ $minor_version -ge 10 ]; then
	    	$CC -g -c $JNI_INCLUDE -I"$PKRU_DIR" -fPIC -o $LIB_DIR/domain_manager.o $PKRU_DIR/domain_manager.c
	    	$CC -g -DREMOVE_NNS_LIMIT -c $JNI_INCLUDE -I"$PKRU_DIR" -fPIC -o $LIB_DIR/memory_map.o $PKRU_DIR/memory_map.c
        	$CC -g -c $JNI_INCLUDE -I"$PKRU_DIR" -fPIC -o $LIB_DIR/pkru_sandbox.o $PKRU_DIR/pkru_sandbox.c
            $CC $CFLAGS -o $LIB_DIR/libpkru.so $LIB_DIR/domain_manager.o $LIB_DIR/memory_map.o \
                $LIB_DIR/pkru_sandbox.o

            LINKER_OPTIONS_PKRU_ISO="-H:NativeLinkerOption=$LIB_DIR/libpkru.so"
    fi
}

function build_nsi {
	HEADER_DIR=$DIR/build/generated/sources/headers/java/main
	C_DIR=$DIR/src/main/c
    PKRU_DIR=$DIR/src/main/c/pkru-sandbox/src
	LIB_DIR=$DIR/build/libs
    build_pkru_sandbox
	$CC -c -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -I"$HEADER_DIR" \
        -I"$PKRU_DIR" -o $LIB_DIR/NativeSandboxInterface.o $C_DIR/NativeSandboxInterface.c
	ar rcs $LIB_DIR/libNativeSandboxInterface.a $LIB_DIR/NativeSandboxInterface.o
}

function build_ni {
    mkdir -p $GRAALVISOR_HOME &> /dev/null
    cd $GRAALVISOR_HOME
    if [[ $JAVA_VERSION == *"17"* ]]; then
        JAVA_17_OPTS="$JAVA_17_OPTS --add-exports org.graalvm.nativeimage.builder/com.oracle.svm.core.posix=ALL-UNNAMED"
	JAVA_17_OPTS="$JAVA_17_OPTS --add-exports org.graalvm.nativeimage.builder/com.oracle.svm.core.posix.headers=ALL-UNNAMED"
        JAVA_17_OPTS="$JAVA_17_OPTS --add-exports org.graalvm.nativeimage.builder/com.oracle.svm.core.c=ALL-UNNAMED"
        JAVA_17_OPTS="$JAVA_17_OPTS --add-exports org.graalvm.nativeimage.builder/com.oracle.svm.core.c.function=ALL-UNNAMED"
        JAVA_17_OPTS="$JAVA_17_OPTS --add-exports org.graalvm.nativeimage.builder/com.oracle.svm.core.jdk=ALL-UNNAMED"
        JAVA_17_OPTS="$JAVA_17_OPTS --add-exports org.graalvm.nativeimage.builder/com.oracle.svm.hosted=ALL-UNNAMED"
        JAVA_17_OPTS="$JAVA_17_OPTS --add-exports org.graalvm.nativeimage.builder/com.oracle.svm.hosted.c=ALL-UNNAMED"
        JAVA_17_OPTS="$JAVA_17_OPTS --add-opens=java.base/java.io=ALL-UNNAMED"
    fi
    $JAVA_HOME/bin/native-image \
        --no-fallback \
        -Dsun.net.httpserver.nodelay=true \
        --enable-url-protocols=http \
        --initialize-at-run-time=com.oracle.svm.graalvisor.utils.JsonUtils \
        $LIBC_OPTION \
        $LINKER_OPTIONS_PKRU_ISO \
        -H:CLibraryPath=$LIB_DIR \
	$JAVA_17_OPTS \
        --features=org.graalvm.argo.graalvisor.sandboxing.NativeSandboxInterfaceFeature \
        -DGraalVisorHost \
        -Dcom.oracle.svm.graalvisor.libraryPath=$DIR/build/resources/main/com.oracle.svm.graalvisor.headers \
        $LANGS \
        -cp $GRAALVISOR_JAR \
        org.graalvm.argo.graalvisor.Main \
        polyglot-proxy \
        -H:+ReportExceptionStackTraces \
        -H:ConfigurationFileDirectories=$DIR/ni-agent-config/native-image
}

if [ -z "$JAVA_HOME" ]
then
    echo "Please set JAVA_HOME first. It should be a GraalVM with native-image available."
    exit 1
else
    eval $(echo "export $(cat $JAVA_HOME/release | grep JAVA_VERSION=)")
    eval $(echo "export $(cat $JAVA_HOME/release | grep GRAALVM_VERSION=)")
fi

if [ -z "$ARGO_HOME" ]
then
    echo "Please set ARGO_HOME first. It should point to a checkout of github.com/graalvm/argo."
    exit 1
fi

cd "$DIR" || {
    echo "Redirection failed!"
    exit 1
}

echo -e "${GREEN}Building graalvisor-lib jar...${NC}"
$ARGO_HOME/graalvisor-lib/build.sh
echo -e "${GREEN}Building graalvisor-lib jar... done!${NC}"

echo -e "${GREEN}Building graalvisor jar...${NC}"
./gradlew clean shadowJar javaProxy
echo -e "${GREEN}Building graalvisor jar... done!${NC}"

echo -e "${GREEN}Building graalvisor native sandbox interface...${NC}"
build_nsi
echo -e "${GREEN}Building graalvisor native sandbox interface... done!${NC}"

echo -e "${GREEN}Building graalvisor Native Image...${NC}"
build_ni
echo -e "${GREEN}Building graalvisor Native Image... done!${NC}"
