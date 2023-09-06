CC := gcc
CFLAGS := -g -Wall

.PHONY: all run clean

all: 
	$(CC) $(CFLAGS) -DSECC_DBG seccomp.c -o test

run:
	./test

clean:
	@echo Cleaning...
	rm -f test
