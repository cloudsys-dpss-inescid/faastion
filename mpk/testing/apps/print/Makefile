PATH_TO_PRELOAD=../../../ld-preload
PATH_TO_JAVASSIST=../../../../native-execution/javassist
PATH_TO_BIN=bin

JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

JAVA_AGENT=$(PATH_TO_JAVASSIST)/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="NativeRedirection"
CLASS_PATH=HelloJNI
ENTRYPOINT=HelloJNI

CC=gcc
INCLUDE_PATH=-I../../.. -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux"
CFLAGS=$(INCLUDE_PATH) -g -fPIC -shared
CXXFLAGS=-Wall -O2 -fno-inline $(CFLAGS)

include ../common.mk

.PHONY: build javassist snippets run gdb clean

build: create_bin preload.so HelloJNI.h libhello.so javassist

HelloJNI.h: HelloJNI.java
	javac -h . HelloJNI.java

libhello.so: HelloJNI.c
	$(CC) $(CFLAGS) -o $(PATH_TO_BIN)/libhello.so HelloJNI.c

javassist: build_javassist_agent create_snippets
	java -javaagent:$(JAVA_AGENT)=$(TOOL):$(CLASS_PATH):$(PATH_TO_BIN) $(ENTRYPOINT)
	
snippets:
	for file in $$(find "snippets" -type f -name "*.c"); do \
		name=$$(basename "$$file" .c); \
		$(CC) $(CXXFLAGS) -o "$(PATH_TO_BIN)/lib$$name.so" "$$file" -L$(PATH_TO_BIN) -lpreload -ldl; \
	done

run:
	export LD_LIBRARY_PATH=$(PATH_TO_BIN)
    LD_PRELOAD=$(PATH_TO_BIN)/libpreload.so java -Djava.library.path=$(PATH_TO_BIN) $(PATH_TO_BIN)/$(ENTRYPOINT)
	
gdb:
	export LD_LIBRARY_PATH=$(PATH_TO_BIN)
	gdb java \
		-ex "set exec-wrapper env 'LD_PRELOAD=$(PATH_TO_BIN)/libpreload.so'" \
		-ex "run -Djava.library.path=$(PATH_TO_BIN) $(PATH_TO_BIN)/$(ENTRYPOINT)"

clean:
	rm -rf bin snippets
	rm -f HelloJNI.h HelloJNI.class
