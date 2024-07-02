#!/bin/bash

. ./.env

echo "JAVA_HOME=$JAVA_HOME"
echo "GRAALVM_HOME=$GRAALVM_HOME"

if [ -z "$JAVA_HOME" ]
then
    echo "JAVA_HOME is not set"
	exit 1
fi

if [ ! -f "$GRAALVM_HOME"/bin/native-image ]
then
    echo "native-image is missing"
	exit 1
fi

function build_app {
	mkdir -p profiles
	cd build
	client_profiles_path=../profiles/client_profiles.iprof
	server_profiles_path=../profiles/server_profiles.iprof
	test -f $client_profiles_path && test -f $server_profiles_path
	profiles_exist=$?
	if [ $profiles_exist -eq 0 ]; then
		echo 'Profiles found, generating image with profiles'
		$GRAALVM_HOME/bin/native-image \
			--no-fallback \
			-H:ReflectionConfigurationFiles=../reflect.json \
			-cp libs/demo-ni-osdcomms-1.0-all.jar\
			-H:+ReportExceptionStackTraces \
			-H:Name=demoosdcomms \
			--pgo=$client_profiles_path,$server_profiles_path \
				com.demo_ni_osdcomms.DemoOsdComms
	else
		echo 'Profiles missing and will be generated in this run, RE-RUN FOR RESULTS WITH PGO!!!'
		$GRAALVM_HOME/bin/native-image \
			--no-fallback \
			-H:ReflectionConfigurationFiles=../reflect.json \
			-cp libs/demo-ni-osdcomms-1.0-all.jar\
			-H:+ReportExceptionStackTraces \
			-H:Name=demoosdcomms \
			--pgo-instrument \
				com.demo_ni_osdcomms.DemoOsdComms
	fi
}

./gradlew clean shadowJar assemble

build_app
