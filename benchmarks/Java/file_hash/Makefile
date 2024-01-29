all: write hash
	
hash: 
	javac FileHash.java
	java FileHash > HashedFile.txt

write: file_generator.py
	python3 file_generator.py 100 100 > test_file.txt

clean:
	rm test_file.txt HashedFile.txt
