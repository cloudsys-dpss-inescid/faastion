PATH_TO_BIN=bin

JAVA_AGENT=$(JAVASSIST_HOME)/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="NativeRedirection"

JNI_INCLUDE=-I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux

CC=gcc
CFLAGS=-Wall -g -fPIC -shared $(JNI_INCLUDE)
SFLAGS=$(CFLAGS) -O0 -fno-inline -I$(PRELOAD_HOME) -DSNI_DBG
