#app = "$(shell pwd)/test0-clib/libhello.so"
#app = "$(shell pwd)/test1-staticvar/libhello.so"
#app = "$(shell pwd)/test2-svm/libhello.so"
#app = "$(shell pwd)/test2-svm-jar/libhello.so"
#app = "$(shell pwd)/test2-svm-gradle/libhello.so"
#app = "$(shell pwd)/test3-svm-truffle-js/libhello.so"
#app = "$(shell pwd)/test3-svm-truffle-py/libhello.so"
#app = "$(shell pwd)/test4-svm-exception/libhello.so"
#app = "$(shell pwd)/test5-svm-truffle-cached-js/libhello.so"
#app = "$(shell pwd)/test5-svm-truffle-cached-py/libhello.so"
#app = "$(shell pwd)/test6-cthreads/libhello.so"
#app = "$(shell pwd)/test7-svm-truffle-function-py/libcompression.so"
#app = "$(shell pwd)/test8-cthreads-syscall/libhello.so"
#app = "$(shell pwd)/test9-cthreads-futex/libhello.so"
#app = "$(shell pwd)/test10-malloc/libhello.so"
#app = "$(shell pwd)/test11-cthreads-mmap/libhello.so"
#app = "$(shell pwd)/test12-svm-http/libhello.so"
#app="${ARGO_HOME}/benchmarks/src/java/gv-hello-world/build/libhelloworld.so"
#app="${ARGO_HOME}/benchmarks/src/javascript/gv-hello-world/build/libhelloworld.so"
#app="${ARGO_HOME}/benchmarks/src/python/gv-hello-world/build/libhelloworld.so"
#app="${ARGO_HOME}/benchmarks/src/python/gv-dynamic-html/build/libdynamichtml.so"
app="${ARGO_HOME}/benchmarks/src/python/gv-thumbnail/build/libthumbnail.so"

ifndef ARGO_HOME
$(error ARGO_HOME is not set (needed to run benchmarks)!)
endif

gcommands = -x gdb-commands.txt

GCC_VERSION := $(shell gcc -dumpversion)
CC = gcc

ifneq ($(shell [ $(GCC_VERSION) -ge 11 ] && echo 1 || echo 0), 1)
    $(warning GCC version must be >= 11 (current: $(GCC_VERSION)))
    CC = gcc-11
    $(info Switching to $(CC))
endif

debug = -O0 -g3

all: *.h *.c
	make -C deps/printf
	$(CC) $(debug) -Wall -c *.c
	$(CC) $(debug) -Wall -o main *.o $(CURDIR)/deps/printf/printf.o
	make -C deps/dlmalloc

tests: all
	make -C test0-clib test
	make -C test1-staticvar test
	make -C test2-svm test
	make -C test2-svm-jar test
	make -C test2-svm-gradle test
	make -C test3-svm-truffle-js test
	make -C test3-svm-truffle-py test
	make -C test4-svm-exception test
	make -C test5-svm-truffle-cached-js test
	make -C test5-svm-truffle-cached-py test
	make -C test6-cthreads test
	make -C test7-svm-truffle-function-py test
	make -C test8-cthreads-syscall test
	make -C test9-cthreads-futex test
	make -C test10-malloc test
	make -C test11-cthreads-mmap test
	make -C test12-svm-http test

benchmarks: all
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/java/gv-hello-world/build/libhelloworld.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/java/gv-file-hashing/build/libfilehashing.so" run
# TODO - classify (process)
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/java/gv-httprequest/build/libhttprequest.so" run
# TODO - videoprocessing (process)
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/javascript/gv-hello-world/build/libhelloworld.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/javascript/gv-dynamic-html/build/libdynamichtml.so" run
# TODO - thumbnail calls native code and it also leaves files behind. We do not support that.
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/javascript/gv-thumbnail/build/libthumbnail.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/javascript/gv-uploader/build/libuploader.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-hello-world/build/libhelloworld.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-compression/build/libcompression.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-uploader/build/libuploader.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-mst/build/libmst.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-pagerank/build/libpr.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-bfs/build/libbfs.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-dna/build/libdna.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-dynamic-html/build/libdynamichtml.so" run
	$(MAKE) app="${ARGO_HOME}/benchmarks/src/python/gv-thumbnail/build/libthumbnail.so" run
# TODO - gv_python_videoprocessing (process)

run-normal: all
	$(CURDIR)/run-env.sh ./main normal $(app) 2>&1 | tee normal.log

strace-normal: all
	$(CURDIR)/run-env.sh strace -f ./main normal $(app) 2>&1 | tee strace-normal.log

gdb-normal: all
	./run-gdb-env.sh ./main normal $(app)

gdb-checkpoint: all
	./run-gdb-env.sh ./main checkpoint $(app) | tee gdb-checkpoint.log

run-checkpoint: all
	rm -f snapshot.mem
	rm -f snapshot.meta
	$(CURDIR)/run-env.sh ./main checkpoint $(app) 2>&1 | tee checkpoint.log

strace-checkpoint: all
	rm -f snapshot.mem
	rm -f snapshot.meta
	$(CURDIR)/run-env.sh strace -f ./main checkpoint $(app) 2>&1 | tee strace-checkpoint.log

run-restore: all
	$(CURDIR)/run-env.sh ./main restore $(app) 2>&1 | tee restore.log

run: run-checkpoint run-restore

run-perf: all
	rm -f snapshot.mem
	rm -f snapshot.meta
	$(CURDIR)/run-env.sh /bin/time -v ./main normal $(app) 2>&1 | tee normal.log
	$(CURDIR)/run-env.sh /bin/time -v ./main checkpoint $(app) 2>&1 | tee checkpoint.log
	$(CURDIR)/run-env.sh /bin/time -v ./main restore $(app) 2>&1 | tee restore.log

gdb-restore: all
	./run-gdb-env.sh ./main restore $(app)

strace-restore: all
	$(CURDIR)/run-env.sh strace -f ./main restore $(app) 2>&1 | tee strace-restore.log

run-minimize: all
	$(CURDIR)/run-env.sh ./main minimize $(app) 2>&1 | tee minimize.log

clean:
	rm -f *.o main *.snap *.log

clean-tests:
	make -C test0-clib clean
	make -C test1-staticvar clean
	make -C test2-svm clean
	make -C test2-svm-jar clean
	make -C test2-svm-gradle clean
	make -C test3-svm-truffle-js clean
	make -C test3-svm-truffle-py clean
	make -C test4-svm-exception clean
	make -C test5-svm-truffle-cached-js clean
	make -C test5-svm-truffle-cached-py clean
	make -C test6-cthreads clean
	make -C test7-svm-truffle-function-py clean
	make -C test8-cthreads-syscall clean
	make -C test9-cthreads-futex clean
