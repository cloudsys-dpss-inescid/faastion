CC = gcc
CFLAGS = -Wall -g -pthread

TARGET = test

.PHONY: build run clean

build:
	$(CC) $(CFLAGS) -o $(TARGET) test_application.c

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)
