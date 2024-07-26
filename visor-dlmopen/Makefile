CC = gcc
CFLAGS = -g3 -Wall

.PHONY: all run clean

all: libapp.so visor

libapp.so: app.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

visor: visor.c pkru_sandbox.c pkru_sandbox.h
	$(CC) $(CFLAGS) -o visor visor.c pkru_sandbox.c

run: all
	strace -f -o strace.log setarch -R env ./visor

run-gdb:
	setarch -R gdb -x gdb-commands.txt --args ./visor

clean:
	rm -f visor *.so *.o *.log

