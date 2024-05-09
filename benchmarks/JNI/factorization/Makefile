export JAVA_HOME=$(HOME)/software/jdk-17.0.10

all: jcompile jnilink jlink jrun

jrun:
	java -cp . -Djava.library.path=. Factorization

jnilink: 
	gcc -c -fPIC -I "$(JAVA_HOME)/include" -I "$(JAVA_HOME)/include/linux" Factorization.c -o Factorization.o

jlink: 
	gcc -shared -fPIC -o libfactors.so Factorization.o -lc

jcompile: Factorization.java
	javac -h . Factorization.java

clean:
	rm libfactors.so Factorization.o Factorization.h Factorization.class	
