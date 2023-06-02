build:
	g++ -Wall -g -I.. -fno-inline -fPIC -shared -o preload.so preload.cpp -lm ../common/libswscommon.a ../erim/liberim.a
	gcc -shared -o libhello.so hello.c
	gcc -o test test.c -ldl

run:
	LD_PRELOAD=./preload.so ./test

clean:
	rm *.so test