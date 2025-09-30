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

# not using full address in restore because last 3 chars of address may change between executions
test: run
	@( \
	    if ! grep -q "malloc'd address 0xa000000014a0" checkpoint.log || \
	    ! grep -q "malloc'd address 0xa00000001" restore.log; then \
		printf ${RED}NOK${NC} && printf " Unexpected addresses for malloc\n"; \
		return 0; \
	    fi; \
	    printf ${GREEN}OK${NC} && printf "\n"; \
	)

clean:
	rm -f *.txt *.o *.so *.snap *.log
