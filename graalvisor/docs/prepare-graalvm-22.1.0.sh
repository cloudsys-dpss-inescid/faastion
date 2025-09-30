#!/bin/bash

if [ -z "$JAVA_HOME" ]
then
    echo "Please set JAVA_HOME first. It you don't have GraalVM locally, download, extract, and set JAVA_HOME."
    echo "Go to: https://www.oracle.com/downloads/graalvm-downloads.html"
    echo "Click on Archived Enterprise Releases > Release Version 22.1.0 > Java 11"
    echo "Download Oracle GraalVM Enterprise Edition Native Image"
    exit 1
fi

# Install GraalVM components.
$JAVA_HOME/bin/gu install native-image python nodejs

# Install graalpy packages in virtual env.
VENV=$JAVA_HOME/graalvisor-python-venv
$JAVA_HOME/bin/graalpython -m venv $VENV
source $VENV/bin/activate
graalpython -m ginstall list
graalpython -m ginstall install numpy
graalpython -m ginstall install Pillow
graalpython -m ginstall install requests
graalpython -m ginstall pypi texttable
graalpython -m ginstall pypi igraph
