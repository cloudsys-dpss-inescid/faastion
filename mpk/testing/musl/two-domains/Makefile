PATH_TO_ERIM=../../../../erim

CC = gcc
CFLAGS = -Wall -O0 -g -fno-inline -I$(PATH_TO_ERIM)/src/erim -I$(PATH_TO_ERIM)/src/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES =$(PATH_TO_ERIM)/bin/common/libswscommon.a $(PATH_TO_ERIM)/bin/erim/liberim.a
LDLIBS = -lm $(LIBRARIES) -ldl

.PHONY: build run clean

build: erim libinc.so test

erim:
	make clean -C $(PATH_TO_ERIM)/src && make -C $(PATH_TO_ERIM)/src

libinc.so:
	musl-gcc -static -g -shared -fPIC -o libinc.so inc.c

test:
	gcc $(CFLAGS) -o test test_application.c $(LDLIBS)

run:
	./test

clean:
	rm -f test libinc.so