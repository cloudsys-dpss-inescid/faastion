CC   = gcc
FLAGS=-pthread

.PHONY: run clean 

run: 
	$(CC) $(FLAGS) -o mpk mpk.c

clean:
	@echo Cleaning...
	rm -f mpk
