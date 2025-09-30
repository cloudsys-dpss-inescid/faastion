app = "$(shell pwd)/libhello.so"
NC ='\033[0m'
RED = '\033[0;31m'
GREEN = '\033[0;32m'

all:
	gcc -O0 -g -c -fPIC hello.c
	gcc -O0 -g -shared -o libhello.so *.o

run-checkpoint:
	rm -f snapshot.mem
	rm -f snapshot.meta
	$(CURDIR)/../run-env.sh ../main checkpoint $(app) > checkpoint.log 2>&1

run-restore:
	$(CURDIR)/../run-env.sh ../main restore $(app) > restore.log 2>&1

run: all run-checkpoint run-restore

test: run
	@( \
	    if ! grep -q "It's contents are: this should be copied" restore.log; then \
		printf ${RED}NOK${NC} && printf " contents of buffer weren't restored\n"; \
		return 0; \
	    fi; \
	    if ! grep -q "Difference between pointers: 1056" restore.log; then \
		printf ${RED}NOK${NC} && printf "Difference between pointers is more than a page\n"; \
		return 0; \
	    fi; \
	    printf ${GREEN}OK${NC} && printf "\n"; \
	)

clean:
	rm -f *.txt *.o *.so *.snap
