run: 
	python3 gen_file.py 34800 300 > hashing_10MB.txt
	python3 gen_file.py 348 300 > hashing_1MB.txt
	python3 gen_file.py 1005 100 > hashing_100KB.txt
	python3 gen_file.py 101 100 > hashing_10KB.txt
	python3 gen_file.py 12 90 > hashing_1KB.txt
