all: run make_file1

env:
	@source env.sh
make_file1:
	@$(MAKE) -f make_file1.mk

run:
	@python3 gen_file.py 38300 300 > hashing.txt

clean:
	@rm -f file_hashing.h file_hashing.class libfile_hashing.so hashing.txt result.txt hashing1.txt hashing2.txt

