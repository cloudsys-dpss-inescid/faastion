#!/bin/bash

function DIR {
    echo "$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
}

JAVA_AGENT=$JAVASSIST_HOME/target/JavassistWrapper-1.0-jar-with-dependencies.jar
JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
TOOL="NativeRedirection"

function build_javassist_agent {
    cd $JAVASSIST_HOME
    mvn package
    cd - &> /dev/null
}

function build_preload_lib {
    make build -C $PRELOAD_HOME
}

function clean_preload {
    make clean -C $PRELOAD_HOME
}

function copy_preload_lib {
    cp $PRELOAD_HOME/build/bin/libpreload.so $(DIR)/$application/bin
}

function build_application {
    cd $(DIR)/$application
    source ./build.sh
    cd - &> /dev/null
}

function clean_application {
    cd $(DIR)/$application
    source ./clean.sh
    cd - &>/dev/null
}

function run_javassist {
    cd $(DIR)/$application
    if [ -n "$class_path" ]; then
        # Get all directories with a *.class file in them and replace '/' with '.'
        output=$(jar -tf ${class_path} | grep -oE ".*\/[^/]+\.class" | sed 's#/[^/]*$##; s#/#.#g' | sort -u)
        # String of packages separated by commas
        printf -v packages '%s,' $output
        packages=${packages%,}

        # Run the java code with the agent.
        java -cp $class_path -javaagent:$JAVA_AGENT=$TOOL:$packages:bin $entrypoint
    else
        java -javaagent:$JAVA_AGENT=$TOOL:$entrypoint:bin $entrypoint
    fi
    cd - &>/dev/null
}

function compile_snippets {
    cd $(DIR)/$application
    for file in $(find "snippets" -type f -name "*.c++"); do
        name=$(basename "$file" .c++)
        g++ -Wall -O2 -g -I. -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/linux" -fno-inline -fPIC -shared bin/preload.so -o "bin/lib$name.so" "$file" -lm ../../../erim/libswscommon.a ../../../erim/liberim.a
    done
    cd - &>/dev/null
}

function run_application {
    cd $(DIR)/$application/bin
    LD_PRELOAD=./libpreload.so java -Djava.library.path=. $entrypoint
    cd - &>/dev/null
}

build_javassist_agent
build_preload_lib

for application in print
do
    echo "Building $application..."
    build_application

    mkdir $(DIR)/$application/snippets
    
    echo "Running javassist for $application..."
    run_javassist

    echo "Copying preload lib to $application..."
    copy_preload_lib

    echo "Compiling snippets for $application..."
    compile_snippets

    echo "Running $application..."
    run_application

    echo "Cleaning..."
    #clean_application
    clean_preload
done


