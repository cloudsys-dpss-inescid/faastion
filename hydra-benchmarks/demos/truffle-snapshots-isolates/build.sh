#!/bin/bash

if [ -z "$JAVA_HOME" ]
then
        echo "Please set JAVA_HOME first. It should be a GraalVM with native-image available."
        exit 1
fi

$JAVA_HOME/bin/javac TruffleEngineCaching.java
$JAVA_HOME/bin/native-image \
	-H:+AuxiliaryEngineCache \
	-H:ReservedAuxiliaryImageBytes=1073741824 \
	-Dorg.graalvm.launcher.home="$JAVA_HOME" \
	--language:js TruffleEngineCaching
