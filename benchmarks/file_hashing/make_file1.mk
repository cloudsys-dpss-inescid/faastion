all: file_hashing.h libfile_hashing.so run

file_hashing.h: file_hashing.java
	@javac -h . file_hashing.java

libfile_hashing.so: file_hashing.cpp
	@gcc -Wall -fPIC -shared -I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/linux" -o libfile_hashing.so file_hashing.cpp -lssl -lcrypto
run:
	@java -Djava.library.path=. file_hashing > result.txt

clean:
	rm -f file_hashing.h file_hashing.class libfile_hashing.so
