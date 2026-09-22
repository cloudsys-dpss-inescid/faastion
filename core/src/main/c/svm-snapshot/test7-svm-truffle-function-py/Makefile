app = "$(shell pwd)/libcompression.so"
NC ='\033[0m'
RED = '\033[0;31m'
GREEN = '\033[0;32m'

libcompression.so: src/main/java/com/compression/Compression.java
	./gradlew clean shadowJar assemble
		#-g -O0
	${JAVA_HOME}/bin/native-image \
		-H:-AllowVMInternalThreads \
		-cp build/libs/compression-1.0-all.jar \
		-Dcom.oracle.svm.graalvisor.polyglotengine.language=python \
		-Dcom.oracle.svm.graalvisor.polyglotengine.entrypoint=main \
		-Dcom.oracle.svm.graalvisor.polyglotengine.source=$(shell pwd)/src/main/python/main.py \
		--initialize-at-build-time=com.compression.Compression \
		--language:python \
		--shared \
		-H:Name=libcompression

all: libcompression.so

run-checkpoint:
	rm -f snapshot.mem
	rm -f snapshot.meta
	$(CURDIR)/../run-env.sh ../main checkpoint $(app) > checkpoint.log 2>&1

run-restore:
	$(CURDIR)/../run-env.sh ../main restore $(app) > restore.log 2>&1

run: all run-checkpoint run-restore

test: run
	-grep --color=always -e warning -e error checkpoint.log restore.log
	if grep "result" restore.log | grep -q ".zip" ; then printf ${GREEN}OK${NC}\\n; else printf ${RED}NOK${NC}\\n; fi

clean:
	rm -f *.txt *.so *.snap *.h *.debug
	rm -f -r build sources bin resources
