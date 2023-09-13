PATH_TO_ERIM = ../../../../erim
ERIM_SRC_DIR = $(PATH_TO_ERIM)/src
ERIM_BIN_DIR = $(PATH_TO_ERIM)/bin

CC = gcc
CFLAGS = -Wall -O0 -g -pthread -fno-inline -I$(ERIM_SRC_DIR)/erim -I$(ERIM_SRC_DIR)/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES = $(ERIM_BIN_DIR)/common/libswscommon.a $(ERIM_BIN_DIR)/erim/liberim.a
LDLIBS = -lm $(LIBRARIES) -ldl

.PHONY: all run clean

all: erim libmmap.so test

erim:
	$(MAKE) -C $(ERIM_SRC_DIR) clean && $(MAKE) -C $(ERIM_SRC_DIR)

libmmap.so: mmap.c
	musl-gcc -static -g -shared -fPIC -o $@ $<

test: seccomp.c
	$(CC) $(CFLAGS) -DSECC_DBG -o $@ $< $(LDLIBS)

run:
	./test

clean:
	rm -f test libmmap.so
	$(MAKE) -C $(ERIM_SRC_DIR) clean

