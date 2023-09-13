PATH_TO_ERIM=../../../../erim

CC = gcc
CFLAGS = -Wall -O0 -g -pthread -fno-inline -I$(PATH_TO_ERIM)/src/erim -I$(PATH_TO_ERIM)/src/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES =$(PATH_TO_ERIM)/bin/common/libswscommon.a $(PATH_TO_ERIM)/bin/erim/liberim.a
LDLIBS = -lm $(LIBRARIES) -ldl

.PHONY: build run clean

build: erim libinc.so liblog.so test

erim:
	$(MAKE) clean -C $(PATH_TO_ERIM)/src && $(MAKE) -C $(PATH_TO_ERIM)/src

libinc.so:
	musl-gcc -static -g -shared -fPIC -o libinc.so inc.c

liblog.so:
	musl-gcc -static -g -shared -fPIC -o liblog.so log.c

test:
	$(CC) $(CFLAGS) -o test test_application.c $(LDLIBS)

run:
	./test

clean:
	rm -f test libinc.so liblog.so