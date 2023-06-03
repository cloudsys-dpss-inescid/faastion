CXX := g++
CC := gcc

CXXFLAGS := -Wall -g -I.. -fno-inline
CFLAGS := -g
LDFLAGS := -ldl

LIBRARIES=../common/libswscommon.a ../erim/liberim.a
LDLIBS= -lm $(LIBRARIES)

.PHONY: build run clean

build: preload.so libhello.so test $(LIBRARIES)

preload.so: preload.cpp
	$(CXX) $(CXXFLAGS) -fPIC -shared -o preload.so preload.cpp $(LDLIBS)

libhello.so: hello.c
	$(CC) $(CFLAGS) -shared -o libhello.so hello.c

test: test.c
	$(CC) $(CFLAGS) -o test test.c $(LDFLAGS)

run: preload.so test
	LD_PRELOAD=./preload.so ./test

clean:
	rm -f *.so test
