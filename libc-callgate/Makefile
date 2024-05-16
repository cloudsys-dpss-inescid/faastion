CC = musl-gcc
CFLAGS = -O0 -g3 -Wall

.PHONY: all run clean

all: libmmap.so libc_callgate.so test

libmmap.so: mmap.c
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $<

test: visor.c
	$(CC) $(CFLAGS) -pthread -o $@ $< -ldl

run:
	strace -f -o strace.log setarch -R env LD_PRELOAD=./libc_callgate.so ./test

run-gdb:
	setarch -R gdb -x gdb-commands.txt --args env LD_PRELOAD=./libc_callgate.so ./test

libc_callgate.so: libc_callgate.c libc_api_processor.py libc_callgate_pkru.h
	find /usr/local/musl/include/ -type f \
		| sed 's/\/usr\/local\/musl\/include\///g' \
		| grep -v "sys/kd.h" \
		| grep -v "sys/vt.h" \
		| grep -v "sys/soundcard.h" \
		| grep -v "linux/kd.h" \
		| grep -v "linux/vt.h" \
		| grep -v "bits/" \
		| awk '{ print "#include <" $$1 ">"}' > libc_headers.c
	$(CC) -E -P libc_headers.c > libc_callgate_generated.c
	ctags -x --c-kinds=p libc_callgate_generated.c \
		| grep -v __REDIR \
		| tr -s ' ' \
		| cut -d' ' -f5- \
		| python libc_api_processor.py >> libc_callgate_generated.c
	$(CC) $(CFLAGS) -fPIC -o libc_callgate.o -c libc_callgate.c
	$(CC) $(CFLAGS) -fPIC -o shared_malloc.o -c shared_malloc.c
	$(CC) $(CFLAGS) -fPIC -o libc_callgate_generated.o -c libc_callgate_generated.c
	$(CC) $(CFLAGS) -shared -fPIC -o libc_callgate.so libc_callgate.o shared_malloc.o libc_callgate_generated.o -ldl

test-libc-callgate:
	$(CC) $(CFLAGS) -o helloworld helloworld.c -ldl
	strace -f -o strace.log setarch -R env LD_PRELOAD=./libc_callgate.so ./helloworld

gdb-libc-callgate:
	setarch -R gdb -x gdb-commands.txt --args env LD_PRELOAD=./libc_callgate.so ./helloworld

clean:
	rm -f test helloworld libc_headers.c libc_callgate_generated.c *.so *.o *.log

