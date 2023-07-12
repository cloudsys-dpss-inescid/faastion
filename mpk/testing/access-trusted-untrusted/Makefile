CC = gcc

CFLAGS = -Wall -g -fno-inline -I../../erim/src/erim -I../../erim/src/common -DERIM_DBG -DERIM_SWAP_STACKS

LIBRARIES =../../erim/bin/common/libswscommon.a ../../erim/bin/erim/liberim.a
LDLIBS = -lm $(LIBRARIES)

TARGET = test

.PHONY: build run clean

build:
	$(CC) $(CFLAGS) -o $(TARGET) test_application.c $(LDLIBS)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)
