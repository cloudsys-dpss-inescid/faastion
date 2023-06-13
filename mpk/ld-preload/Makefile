CXX := g++
CXXFLAGS := -Wall -g -I.. -fno-inline

LIBRARIES=../erim/libswscommon.a ../erim/liberim.a
LDLIBS=-lm $(LIBRARIES)

.PHONY: build clean

build: preload.cpp preload.h
	$(CXX) $(CXXFLAGS) -fPIC -shared -o libpreload.so preload.cpp $(LDLIBS)

clean:
	rm -f libpreload.so
