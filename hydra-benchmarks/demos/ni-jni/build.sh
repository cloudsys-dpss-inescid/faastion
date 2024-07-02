#!/bin/bash

if [ -z "$JAVA_HOME" ]
then
        echo "JAVA_HOME is not set. Recommended JVM: graalvm-ee-java11-22.1.0"
fi

if [ ! -f "JVips.jar" ];
then
	wget https://github.com/criteo/JVips/releases/download/8.12.2-f9dc8c9/JVips.jar
fi

$JAVA_HOME/bin/javac -cp JVips.jar SimpleExample.java

$JAVA_HOME/bin/java -agentlib:native-image-agent=config-output-dir=agent-output -cp JVips.jar:. SimpleExample

$JAVA_HOME/bin/native-image \
	--no-fallback \
	-H:ConfigurationFileDirectories=agent-output \
	-cp JVips.jar:. \
	-H:+ReportExceptionStackTraces \
	-H:Name=demonijni \
	SimpleExample
