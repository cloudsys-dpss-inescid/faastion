CC := gcc

CFLAGS := -g
LDFLAGS := -ldl

TARGET = test

.PHONY: build run clean

build: preload.so libhello.so target

preload.so:
	make -C ../../ld-preload
	mv ../../ld-preload/libpreload.so .
	make clean -C ../../ld-preload

libhello.so: hello.c
	$(CC) $(CFLAGS) -shared -o libhello.so hello.c

target: test.c
	$(CC) $(CFLAGS) -o $(TARGET) test.c $(LDFLAGS)

run:
	LD_PRELOAD=./libpreload.so ./$(TARGET)

gdb: preload.so libhello.so target
	gdb ./$(TARGET) -ex "set exec-wrapper env 'LD_PRELOAD=./libpreload.so'"

clean:
	rm -f *.so $(TARGET)
