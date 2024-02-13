export JAVA_HOME=$(HOME)/software/jdk-17.0.10

all: jcompile jnilink jlink jrun

jrun:
	java -cp . -Djava.library.path=. matrix_multiplication

jnilink: 
	gcc -c -fPIC -I "$(JAVA_HOME)/include" -I "$(JAVA_HOME)/include/linux" matrix_multiplication.c -o matrix_multiplication.o

jlink: 
	gcc -shared -fPIC -o libnative.so matrix_multiplication.o -lc

jcompile: 
	javac -h . matrix_multiplication.java

clean:
	rm libnative.so matrix_multiplication.o matrix_multiplication.h matrix_multiplication.class	
