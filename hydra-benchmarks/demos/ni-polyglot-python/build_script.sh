#!/bin/bash

if [ -z "$JAVA_HOME" ]
then
	echo "JAVA_HOME is not set. Recommended JVM: graalvm-ee-java11-22.1.0"
fi

function build_app {
	cd build
	$JAVA_HOME/bin/native-image \
		--no-fallback \
		--language:python \
		-cp libs/demo-polyglot-1.0-all.jar\
		-H:+ReportExceptionStackTraces \
		-H:Name=demopolyglot \
                com.demo_polyglot.DemoPolyglot
}

./gradlew clean shadowJar assemble

$JAVA_HOME/bin/java -cp build/libs/demo-polyglot-1.0-all.jar com.demo_polyglot.DemoPolyglot

#build_app
