CLASS_PATH=HelloJNI
ENTRYPOINT=HelloJNI

include ../flags.mk
include ../common.mk

.PHONY: all snippets run gdb clean

all: requirements HelloJNI.h libhello.so javassist snippets

# Build process
HelloJNI.h: HelloJNI.java
	javac -h . HelloJNI.java

libhello.so: HelloJNI.c
	musl-gcc -static $(CFLAGS) -o $(PATH_TO_BIN)/libhello.so HelloJNI.c

# Clean process
clean:
	rm -rf bin snippets
	rm -f HelloJNI.h HelloJNI.class
