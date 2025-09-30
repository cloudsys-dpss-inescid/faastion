app = "$(shell pwd)/libhello.so"
NC ='\033[0m'
RED = '\033[0;31m'
GREEN = '\033[0;32m'

libhello.so: src/main/java/com/hello_world/HelloWorld.java
	mkdir build &> /dev/null
	${JAVA_HOME}/bin/javac -d build src/main/java/com/hello_world/HelloWorld.java
	${JAVA_HOME}/bin/jar --create --file build/hw.jar --manifest src/main/resources/META-INF/MANIFEST.MF -C build com
	${JAVA_HOME}/bin/native-image -g -O0 --enable-url-protocols=http -H:-AllowVMInternalThreads -cp build --shared -H:Name=libhello com.hello_world.HelloWorld

all: libhello.so

run-checkpoint:
	rm -f snapshot.mem
	rm -f snapshot.meta
	$(CURDIR)/../run-env.sh ../main checkpoint $(app) > checkpoint.log 2>&1

run-restore:
	$(CURDIR)/../run-env.sh ../main restore $(app) > restore.log 2>&1

run: all run-checkpoint run-restore

test: run
	-grep --color=always -e warning -e error checkpoint.log restore.log
	-grep "Downloaded" restore.log | wc -l > result.log
	if grep -q "10000" result.log ; then printf ${GREEN}OK${NC}\\n; else printf ${RED}NOK${NC}\\n; fi

clean:
	rm -f *.txt *.so *.snap *.h *.debug *.log
	rm -f -r sources build
