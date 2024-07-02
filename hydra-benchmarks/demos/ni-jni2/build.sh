#!/bin/bash

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"

mkdir -p build/lib
mkdir -p build/include

$JAVA_HOME/bin/javac -h build/include my/app/HelloWorld.java
$JAVA_HOME/bin/javac -cp $HOME/.m2/repository/org/graalvm/nativeimage/svm/21.0.0/svm-21.0.0.jar my/app/HelloWorld.java my/app/NativeFeature.java
gcc -c -I "$JAVA_HOME/include" -I "$JAVA_HOME/include/linux" -Ibuild/include -o build/lib/native.o Native.c
ar rcs build/lib/libNative.a build/lib/native.o
$JAVA_HOME/bin/native-image -cp . -H:CLibraryPath=$DIR/build/lib my.app.HelloWorld
