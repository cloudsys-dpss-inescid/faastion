#!/bin/bash

# Note, we are using a Java 8 runtime so we need to compile with Java 8.
JAVA_HOME=/usr/lib/jvm/openjdk1.8.0_302-jvmci-20.3-b23
$JAVA_HOME/bin/javac -cp gson-2.8.6.jar Hello.java
$JAVA_HOME/bin/jar cvf hello.jar Hello.class
body=$(base64 --wrap=0 hello.jar)
echo "{ \"value\": { \"binary\": \"True\", \"main\": \"Hello\", \"code\": \"$body\" } }" > init.json
