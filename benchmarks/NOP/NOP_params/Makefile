NUM_RUNS := 100

run_multiple_times: all 
	for i in $$(seq 1 $(NUM_RUNS)); do \
		java -Djava.library.path=. Nop >> visualization/exec.txt; \
	done

all: Nop.h libNop.so

Nop.h: Nop.java
	@javac -h . Nop.java

libNop.so: Nop.cpp
	@gcc -Wall -fPIC -shared -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux" -o libNop.so Nop.cpp 

run:
	@java -Djava.library.path=. Nop

clean:
	@rm -f Nop.h Nop.class libNop.so visualization/exec.txt
