PATH_TO_BIN=bin

JAVA_AGENT=$(JAVASSIST_HOME)/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="NativeRedirection"

JNI_INCLUDE=-I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux
ERIM_INCLUDE=-I$(ERIM_HOME)/src/erim -I$(ERIM_HOME)/src/common

CC=gcc
CFLAGS=-Wall -g -fPIC -shared $(JNI_INCLUDE)
SFLAGS=$(CFLAGS) -O0 -fno-inline $(ERIM_INCLUDE) -I$(PRELOAD_HOME)
