CXX := g++
CC := gcc

CXXFLAGS := -Wall -g -I.. -fno-inline
CFLAGS := -g
LDFLAGS := -ldl

LIBRARIES =../common/libswscommon.a ../erim/liberim.a
LDLIBS = -lm $(LIBRARIES)

TARGET = test

.PHONY: build run clean

build: preload.so libhello.so target

preload.so: preload.cpp
	$(CXX) $(CXXFLAGS) -fPIC -shared -o preload.so preload.cpp $(LDLIBS)

libhello.so: hello.c
	$(CC) $(CFLAGS) -shared -o libhello.so hello.c

target: test.c
	$(CC) $(CFLAGS) -o $(TARGET) test.c $(LDFLAGS)

run: preload.so target
	LD_PRELOAD=./preload.so ./$(TARGET)

clean:
	rm -f *.so $(TARGET)
