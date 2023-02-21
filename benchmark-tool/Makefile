CC   = g++
FLAGS=-pthread

.PHONY: all clean run

all: benchmark

benchmark: utils/operations.o main.o 
	$(CC) $(FLAGS) -o benchmark utils/operations.o main.o

utils/operations.o: utils/operations.cpp utils/operations.h
	$(CC) $(FLAGS) -o utils/operations.o -c utils/operations.cpp

main.o: main.cpp utils/operations.h utils/timer.h
	$(CC) $(FLAGS) -o main.o -c main.cpp

clean:
	@echo Cleaning...
	rm -f utils/*.o *.o benchmark

run: benchmark
	./benchmark
