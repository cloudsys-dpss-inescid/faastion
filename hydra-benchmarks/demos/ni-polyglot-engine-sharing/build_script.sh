#!/bin/bash

if [ -z "$JAVA_HOME" ]
then
	echo "JAVA_HOME is not set. Recommended JVM: graalvm-ee-java11-22.1.0"
	exit 1
fi

function build_app {
	cd build
	$JAVA_HOME/bin/native-image \
		--language:python \
		--language:js \
		-cp libs/demo-polyglot-engine-sharing-1.0-all.jar \
		-H:Name=demopolyglot \
                com.demo_polyglot.DemoPolyglot
}

./gradlew clean shadowJar assemble

build_app
