PATH_TO_ERIM = ../../erim
ERIM_SRC_DIR = $(PATH_TO_ERIM)/src
ERIM_BIN_DIR = $(PATH_TO_ERIM)/bin

CC = gcc
CFLAGS = -Wall -O0 -g -pthread -fno-inline -I$(ERIM_SRC_DIR)/erim -I$(ERIM_SRC_DIR)/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES = $(ERIM_BIN_DIR)/common/libswscommon.a $(ERIM_BIN_DIR)/erim/liberim.a
LDLIBS = -lm $(LIBRARIES) -ldl

.PHONY: all build run clean

all: erim libs test

erim:
	$(MAKE) -C $(ERIM_SRC_DIR) clean && $(MAKE) -C $(ERIM_SRC_DIR)

libs: libabs.so libadd.so libcapitalize.so libeven.so libhello.so libinc.so liblog.so libmax.so libmin.so libmultiply.so libnegate.so libnegative.so libpositive.so libsquare.so libsubtract.so

lib%.so: %.c
	musl-gcc -static -g -shared -fPIC -o $@ $<

test: test_application.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

build: all

run: test
	./test

clean:
	rm -f lib*.so test
	$(MAKE) -C $(ERIM_SRC_DIR) clean
