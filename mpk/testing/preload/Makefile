PRELOAD_HOME = /home/cidra/faastion/mpk/ld-preload
ERIM_HOME = /home/cidra/faastion/erim
PATH_TO_BIN = bin

CC = gcc
INCLUDES = -I$(ERIM_HOME)/src/erim -I$(ERIM_HOME)/src/common -I$(PRELOAD_HOME)/src
CFLAGS = -Wall -g -fno-inline -fPIC -shared -DERIM_DBG -DERIM_SWAP_STACKS

all: create_bin libpreload.so
	$(CC) $(CFLAGS) $(INCLUDES) -o $(PATH_TO_BIN)/libtest.so test_application.c -L$(PATH_TO_BIN) -lpreload

create_bin:
	mkdir -p bin

libpreload.so:
	make -C $(PRELOAD_HOME)/src
	mv $(PRELOAD_HOME)/src/build/bin/libpreload.so $(PATH_TO_BIN)
	make clean -C $(PRELOAD_HOME)/src

clean:
	rm -rf bin