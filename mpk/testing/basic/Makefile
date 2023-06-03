CC = gcc

CFLAGS = -Wall -g -fno-inline -I..

LIBRARIES =../common/libswscommon.a ../erim/liberim.a
LDLIBS = -lm $(LIBRARIES)

TARGET = test

.PHONY: build run clean

build:
	$(CC) $(CFLAGS) -o $(TARGET) test_application.c $(LDLIBS)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)
