PATH_TO_ERIM=../../erim

CC = gcc
CFLAGS = -Wall -g -fno-inline -I$(PATH_TO_ERIM)/src/erim -I$(PATH_TO_ERIM)/src/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES =$(PATH_TO_ERIM)/bin/common/libswscommon.a $(PATH_TO_ERIM)/bin/erim/liberim.a
LDLIBS = -lm $(LIBRARIES)

TARGET = test

.PHONY: build run clean

build: erim
	$(CC) $(CFLAGS) -o $(TARGET) test_application.c $(LDLIBS)

erim:
	make clean -C $(PATH_TO_ERIM)/src && make -C $(PATH_TO_ERIM)/src

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)
