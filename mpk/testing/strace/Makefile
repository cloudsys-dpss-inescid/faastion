.PHONY: all run clean

all:
	gcc -pthread -o test test_application.c

run:
	strace -f  ./test

clean:
	@echo Cleaning...
	rm -f test
