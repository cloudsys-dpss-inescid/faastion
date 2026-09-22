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
	-grep --color=always -e warning -e error checkpoint.log restore.log
	if grep -q "background thread" restore.log ; then printf ${GREEN}OK${NC}\\n; else printf ${RED}NOK${NC}\\n; fi

clean:
	rm -f *.txt *.o *.so *.snap *.log
