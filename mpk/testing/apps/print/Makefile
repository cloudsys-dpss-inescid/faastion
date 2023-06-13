PATH_TO_PRELOAD=../../../ld-preload
PATH_TO_JAVASSIST=../../../../native-execution/javassist
PATH_TO_BIN=bin

JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

JAVA_AGENT=$(PATH_TO_JAVASSIST)/target/JavassistWrapper-1.0-jar-with-dependencies.jar
TOOL="NativeRedirection"
CLASS_PATH=HelloJNI
ENTRYPOINT=HelloJNI

CC=gcc
INCLUDE_PATH=-I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux"
CFLAGS=$(INCLUDE_PATH) -g -fPIC -shared

CXX=g++
CXXFLAGS=-Wall -O2 -fno-inline $(CFLAGS)

LIBRARIES=../../../erim/libswscommon.a ../../../erim/liberim.a
LDLIBS=-lm $(LIBRARIES)

.PHONY: create_snippets create_bin build_javassist_agent build preload.so HelloJNI.h libhello.so javassist snippets run clean

create_snippets:
	mkdir -p snippets

create_bin:
	mkdir -p bin

build_javassist_agent:
	cd $(PATH_TO_JAVASSIST) && mvn -q package

build: create_bin preload.so HelloJNI.h libhello.so javassist

preload.so:
	make build -C $(PATH_TO_PRELOAD)
	mv $(PATH_TO_PRELOAD)/libpreload.so $(PATH_TO_BIN)

HelloJNI.h: HelloJNI.java
	javac -h . HelloJNI.java

libhello.so: HelloJNI.c
	$(CC) $(CFLAGS) -o $(PATH_TO_BIN)/libhello.so HelloJNI.c

javassist: build_javassist_agent create_snippets HelloJNI.h HelloJNI.class
	java -javaagent:$(JAVA_AGENT)=$(TOOL):$(CLASS_PATH):$(PATH_TO_BIN) $(ENTRYPOINT)

snippets:
	for file in $$(find "snippets" -type f -name "*.c++"); do \
		name=$$(basename "$$file" .c++); \
		$(CXX) $(CXXFLAGS) -o "$(PATH_TO_BIN)/lib$$name.so" "$$file" -L$(PATH_TO_BIN) -lpreload $(LDLIBS); \
	done

run:
    LD_PRELOAD=$(PATH_TO_BIN)/libpreload.so java -Djava.library.path=$(PATH_TO_BIN) $(PATH_TO_BIN)/$(ENTRYPOINT)

clean:
	rm -rf bin snippets
	rm -f HelloJNI.h HelloJNI.class
